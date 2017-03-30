#include "Message.hpp"
#include <fstream>
#include <adbase/Lua.hpp>

namespace app {
// {{{ Message::Message()

Message::Message(AdbaseConfig* configure, adbase::lua::Engine* engine):
	_configure(configure),
	_engine(engine)	{
	adbase::metrics::Metrics::buildGauges("message", "queue.size", 1, [this](){
		return _queue.getSize();
	});
}

// }}}
// {{{ Message::~Message()

Message::~Message() {
}

// }}}
// {{{ void Message::start()

void Message::start() {
	ThreadPtr callThread(new std::thread(std::bind(&Message::call, this, std::placeholders::_1), nullptr), &Message::deleteThread);
	LOG_DEBUG << "Create call lua thread success";
	Threads.push_back(std::move(callThread));
}

// }}}
// {{{ void Message::stop()

void Message::stop() {
	MessageItem newItem;
	newItem.mask = 0x01;
	newItem.partId = 0;
	newItem.offset = 0;
	_queue.push(newItem);
}

// }}}
// {{{ void Message::reload()

void Message::reload() {
}

// }}}
// {{{ void Message::call()

void Message::call(void *) {
	int batchNum = _configure->consumerBatchNumber;
	while(true) {
		MessageItem item;
		_queue.waitPop(item);
		if (item.mask == 0x01) {
			break;
		}

		addLuaMessage(item);

		if (static_cast<int>(_queue.getSize()) > batchNum) {
			bool ret = false;
			bool exit = false;
			do {
				MessageItem itemBatch;
				ret = _queue.tryPop(itemBatch);
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
// {{{ void Message::saveMessage()

void Message::saveMessage() {
	//MessageItem item;
	//while (!_queue.empty()) { // 获取队列中的数据
	//}

	////std::lock_guard<std::mutex> lk(_mut);
	//std::ofstream ofs(_configure->messageSwp.c_str(), std::ofstream::app | std::ofstream::binary);
	////for (auto &t : _luaMessages) {
	////	adbase::Buffer dumpBuffer;
	////	serialize(dumpBuffer, t);
	////	ofs.write(dumpBuffer.peek(), dumpBuffer.readableBytes());
	////}
	//LOG_INFO << "Save message, count " << _luaMessages.size();
	//ofs.flush();
	//ofs.close();
}

// }}}
// {{{ void Message::loadMessage()

void Message::loadMessage() {
	std::lock_guard<std::mutex> lk(_mut);
	std::ifstream ifs(_configure->messageSwp.c_str(), std::ios_base::in | std::ios_base::binary);
	if (!ifs.good() || !ifs.is_open()) {
		return;
	}

	while (true) {
		adbase::Buffer loadBuffer;
		uint32_t headerSize = 4 * static_cast<uint32_t>(sizeof(uint32_t));
		char lenBuf[headerSize];
		memset(lenBuf, 0, headerSize);
		ifs.read(lenBuf, headerSize);
		if (!ifs.good() || ifs.gcount() != headerSize) {
			break;
		}
		loadBuffer.append(lenBuf, headerSize);

		uint64_t offset = loadBuffer.readInt64();
		uint32_t partId = loadBuffer.readInt32();
		uint32_t messageSize = loadBuffer.readInt32();

        std::unique_ptr<char[]> data(new char[messageSize]);
		ifs.read(data.get(), messageSize);
		if (!ifs.good() || ifs.gcount() != messageSize) {
			break;
		}
		adbase::Buffer message;
		message.append(data.get(), messageSize);

		MessageItem newItem;
		newItem.partId = partId;
		newItem.offset = offset;
		newItem.message = message;

		_queue.push(newItem);
	}

	ifs.close();
	std::string bakPathName = _configure->messageSwp + ".bak";
	if (0 != rename(_configure->messageSwp.c_str(), bakPathName.c_str())) {
		LOG_INFO << "Rename error file to bak fail.";
	}
}

// }}}
// {{{ void Message::serialize()

void Message::serialize(adbase::Buffer& buffer, MessageItem& item) {
	buffer.appendInt64(item.offset);
	buffer.appendInt32(item.partId);
	size_t messageSize = item.message.readableBytes();
	buffer.appendInt32(static_cast<uint32_t>(messageSize));
	buffer.append(item.message.peek(), messageSize);
}

// }}}
// {{{ void Message::callMessage()

void Message::callMessage() {
	std::lock_guard<std::mutex> lk(_mut);
	bool isCall = _engine->runFile(_configure->consumerScriptName.c_str());
	if (isCall) {
		_luaMessages.clear();
	}
}

// }}}
// {{{ MessageToLua Message::getMessage()

MessageToLua Message::getMessage() {
	MessageToLua ret;
	for	(auto &t : _luaMessages) {
		std::string id = std::to_string(t.second.partId) + "_" + std::to_string(t.second.offset);
		std::string message = t.second.message.retrieveAllAsString();
		std::pair<std::string, std::string> item(id, message);
		ret.push_back(item);
	}
	return ret;
}

// }}}
// {{{ bool Message::push()

bool Message::push(MessageItem& item) {
	if (item.message.readableBytes()) {
		_queue.push(item);
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
	std::lock_guard<std::mutex> lk(_mut);
	_luaMessages[convertKey(item)] = item;
}

// }}}
// {{{ const std::string Message::convertKey()

const std::string Message::convertKey(MessageItem& item) {
	std::string key = std::to_string(item.partId) + "_" + std::to_string(item.offset);
	return key;
}

// }}}
}
