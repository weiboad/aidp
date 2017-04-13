#include "Message.hpp"
#include "Storage.hpp"
#include <fstream>
#include <adbase/Lua.hpp>

namespace app {
thread_local std::unique_ptr<adbase::lua::Engine> messageLuaEngine;
thread_local std::unordered_map<std::string, MessageItem> messageLuaMessages;

// {{{ Message::Message()

Message::Message(AdbaseConfig* configure, Storage* storage): _configure(configure), _storage(storage) {
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
	for (int i = 0; i < _configure->consumerThreadNumber; i++) {
		_queues[i] = new MessageQueue;	
		std::string key = "queue" + std::to_string(i) + ".size";
		adbase::metrics::Metrics::buildGauges("message", key, 1, [this, i](){
				return _queues[i]->getSize();
		});
		ThreadPtr callThread(new std::thread(std::bind(&Message::call, this, std::placeholders::_1), i), &Message::deleteThread);
		LOG_DEBUG << "Create call lua thread success";
		Threads.push_back(std::move(callThread));
	}

}

// }}}
// {{{ void Message::stop()

void Message::stop() {
	for (auto &t : _queues) {
		MessageItem newItem;
		newItem.mask = 0x01;
		newItem.partId = 0;
		newItem.offset = 0;
		t.second->push(newItem);
	}
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

	int batchNum = _configure->consumerBatchNumber;
	while(true) {
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
			} while(batchNum--);

			if (exit) {
				break;
			}
		}
		callMessage();
	}
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

	typedef std::function<void (std::list<std::string>)> RollBackMessageFn;
	RollBackMessageFn rollbackFn = std::bind(&app::Message::rollback, this, std::placeholders::_1);
	clazz.addMethod("rollback", rollbackFn);

	_storage->bindClass(messageLuaEngine.get());
}

// }}}
// {{{ void Message::callMessage()

void Message::callMessage() {
	bool isCall = messageLuaEngine->runFile(_configure->consumerScriptName.c_str());
	if (isCall) {
		messageLuaMessages.clear();
	}
}

// }}}
// {{{ MessageToLua Message::getMessage()

MessageToLua Message::getMessage() {
	MessageToLua ret;
	for	(auto &t : messageLuaMessages) {
		std::list<std::string> item;
		std::string id = convertKey(t.second);
		std::string message = t.second.message.retrieveAllAsString();
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
            int processQueueNum = item.partId % _configure->consumerThreadNumber;
            _queues[processQueueNum]->push(item);
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

	if (static_cast<int>(_queues[processQueueNum]->getSize()) > _configure->consumerMaxNumber) {
		return false;
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
}
