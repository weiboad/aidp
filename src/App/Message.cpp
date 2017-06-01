#include "Message.hpp"
#include "Storage.hpp"
#include "Metrics.hpp"
#include <fstream>
#include <adbase/Lua.hpp>

namespace app {
thread_local std::unique_ptr<adbase::lua::Engine> messageLuaEngine;
thread_local std::unordered_map<std::string, MessageItem> messageLuaMessages;

// {{{ Message::Message()

Message::Message(AdbaseConfig* configure, Storage* storage, Metrics* metrics): _configure(configure), 
    _storage(storage),
    _metrics(metrics),
    _isRunning(false) {
}

// }}}
// {{{ Message::~Message()

Message::~Message() {
	for(auto &t : _queues) {
		if (t.second != nullptr) {
			delete t.second;	
			t.second = nullptr;
		}
	}
}

// }}}
// {{{ void Message::start()

void Message::start() {
    _threadNumber = _configure->consumerThreadNumber;
    _isRunning = true;
    _luaProcessTimer = adbase::metrics::Metrics::buildTimers("message", "process", 60000);
	for (int i = 0; i < _configure->consumerThreadNumber; i++) {
		_queues[i] = new MessageQueue;	
		std::string key = "queue" + std::to_string(i) + ".size";
		adbase::metrics::Metrics::buildGauges("message", key, 1000, [this, i](){
				return _queues[i]->getSize();
		});
		ThreadPtr callThread(new std::thread(std::bind(&Message::call, this, std::placeholders::_1), i), &Message::deleteThread);
		LOG_DEBUG << "Create call lua thread success";
		Threads.push_back(std::move(callThread));
	}
    std::string messageName = _configure->messageDir + "/message_error";
    _logger = std::shared_ptr<adbase::AsyncLogging>(new adbase::AsyncLogging(messageName, _configure->messageRollSize));
    _logger->start();

}

// }}}
// {{{ void Message::stop()

void Message::stop() {
	for (auto &t : _queues) {
		MessageItem newItem;
		newItem.mask = 0x01;
		newItem.partId = 0;
		newItem.offset = 0;
		newItem.tryNum = 0;
		t.second->push(newItem);
	}
    _isRunning = false;
    std::unique_lock<std::mutex> lk(_mut);
    _dataCond.wait(lk, [this]{return (_threadNumber == 0);});
}

// }}}
// {{{ void Message::reload()

void Message::reload() {
}

// }}}
// {{{ void Message::call()

void Message::call(int i) {
	// 初始化 lua 引擎
	initLua();
    std::string key = "lua.message.map" + std::to_string(i) + ".size";
    adbase::metrics::Metrics::buildGauges("message", key, 1000, [this, i](){
        return messageLuaMessages.size();
    });

	int batchNum = _configure->consumerBatchNumber;
	while(_isRunning) {
		MessageItem item;
		_queues[i]->waitPop(item);
		if (item.mask == 0x01) {
			break;
		}

		addLuaMessage(item);

		if (static_cast<int>(_queues[i]->getSize()) > batchNum) {
			bool ret = false;
			bool exit = false;
			do {
				MessageItem itemBatch;
				ret = _queues[i]->tryPop(itemBatch);
				if (item.mask == 0x01) {
					exit = true;
					continue;
				}
				if (!ret) {
					break;
				}
				addLuaMessage(itemBatch);
                batchNum--;
			} while(batchNum > 0);

			if (exit) {
				break;
			}
		}
		callMessage();
	}

    for (auto &t : messageLuaMessages) {
        saveMessage(t.second);
    }

    while(_queues[i]->getSize()) {
        MessageItem item;
		bool ret = _queues[i]->tryPop(item);
        if (!ret) {
            break;
        }
        saveMessage(item);
    }

    std::unique_lock<std::mutex> lk(_mut);
    _threadNumber--;
    _dataCond.notify_all();
}

// }}}
// {{{ void Message::initLua()

void Message::initLua() {
    std::unique_ptr<adbase::lua::Engine> engine(new adbase::lua::Engine());
    messageLuaEngine = std::move(engine);
	messageLuaEngine->init();
	messageLuaEngine->addSearchPath(_configure->luaScriptPath, true);

	adbase::lua::BindingClass<app::Message> clazz("message", "aidp", messageLuaEngine->getLuaState());
	typedef std::function<MessageToLua()> GetMessageFn;
	GetMessageFn getMessageFn = std::bind(&app::Message::getMessage, this);
	clazz.addMethod("get", getMessageFn);

	typedef std::function<int (std::list<std::string>)> RollBackMessageFn;
	RollBackMessageFn rollbackFn = std::bind(&app::Message::rollback, this, std::placeholders::_1);
	clazz.addMethod("rollback", rollbackFn);

	_storage->bindClass(messageLuaEngine.get());
	_metrics->bindClass(messageLuaEngine.get());
}

// }}}
// {{{ void Message::callMessage()

void Message::callMessage() {
    adbase::metrics::Timer timer;
    timer.start();
	bool isCall = messageLuaEngine->runFile(_configure->consumerScriptName.c_str());
	if (!isCall) {
        for (auto &t : messageLuaMessages) { 
            saveMessage(t.second);
        }
    }
	messageLuaMessages.clear();
    double time = timer.stop();
    if (_luaProcessTimer != nullptr) {
        _luaProcessTimer->setTimer(time);
    }
}

// }}}
// {{{ MessageToLua Message::getMessage()

MessageToLua Message::getMessage() {
	MessageToLua ret;
	for	(auto &t : messageLuaMessages) {
		std::list<std::string> item;
		std::string id = convertKey(t.second);
        adbase::Buffer tmp = t.second.message;
		std::string message = tmp.retrieveAllAsString();
		item.push_back(id);
		item.push_back(t.second.topicName);
		item.push_back(message);
		ret.push_back(item);
	}
	return ret;
}

// }}}
// {{{ int Message::rollback()

int Message::rollback(std::list<std::string> ids) {
    int count = 0;
	for	(auto &t : ids) {
        if (messageLuaMessages.find(t) != messageLuaMessages.end()) {
            MessageItem item = messageLuaMessages[t];
            saveMessage(item);
            count++;
        }
	}
    return count;
}

// }}}
// {{{ bool Message::push()

bool Message::push(MessageItem& item) {
	int processQueueNum = item.partId % _configure->consumerThreadNumber;
	if (item.message.readableBytes()) {
		_queues[processQueueNum]->push(item);
	}

	return true;
}

// }}}
// {{{ bool Message::queueCheck()

bool Message::queueCheck() {
	for(auto &t : _queues) {
        if (static_cast<int>(t.second->getSize()) > _configure->consumerMaxNumber) {
            return false;
        }
    }

    return true;
}

// }}}
// {{{ void Message::deleteThread()

void Message::deleteThread(std::thread *t) {
	t->join();
	delete t;
}

// }}}
// {{{ void Message::addLuaMessage()

void Message::addLuaMessage(MessageItem& item) {
	messageLuaMessages[convertKey(item)] = item;
}

// }}}
// {{{ const std::string Message::convertKey()

const std::string Message::convertKey(MessageItem& item) {
	std::string key = std::to_string(item.partId) + "_" + std::to_string(item.offset) + "_" + item.topicName;
	return key;
}

// }}}
// {{{ void Message::serialize()

void Message::serialize(adbase::Buffer& buffer, MessageItem& item) {
    buffer.appendInt32(item.partId);
    buffer.appendInt32(item.tryNum);
    buffer.appendInt32(static_cast<uint32_t>(item.topicName.size()));
    buffer.append(item.topicName);

    size_t messageSize = item.message.readableBytes();
    buffer.appendInt32(static_cast<uint32_t>(messageSize));
    buffer.append(item.message.peek(), messageSize);
}

// }}}
// {{{ void Message::saveMessage()

void Message::saveMessage(MessageItem& item) {
    if (item.tryNum < _configure->consumerTryNumber) {
        int processQueueNum = item.partId % _configure->consumerThreadNumber;
        item.tryNum = item.tryNum + 1;
        _queues[processQueueNum]->push(item);
    } else {
        adbase::Buffer buffer;
        serialize(buffer, item);
        if (_logger) {
            _logger->append(buffer.peek(), static_cast<int>(buffer.readableBytes()));
        }
        if (_errorMessageCounters.find(item.topicName) == _errorMessageCounters.end()) {
            std::string metricName = "error." + item.topicName;
            _errorMessageCounters[item.topicName] = adbase::metrics::Metrics::buildCounter("message", metricName);
        }

        if (_errorMessageCounters[item.topicName] != nullptr) {
            _errorMessageCounters[item.topicName]->add(1);
        }
    }
}

// }}}
}
