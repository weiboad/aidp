#ifndef AIDP_APP_MESSAGE_HPP_
#define AIDP_APP_MESSAGE_HPP_

#include <adbase/Utility.hpp>
#include <adbase/Logging.hpp>
#include <adbase/Lua.hpp>
#include <adbase/Metrics.hpp>
#include <unordered_map>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <list>
#include <limits.h>
#include "AdbaseConfig.hpp"

namespace app {
class Storage;
class Metrics;
// message queue
typedef struct MessageItem {
    int partId;
	int mask; // 0x00 normal, 0x01 stop
    uint64_t offset;
    int tryNum;
	std::string topicName;
    adbase::Buffer message;
} MessageItem;

typedef adbase::Queue<MessageItem> MessageQueue;
typedef std::list<std::list<std::string>> MessageToLua;


class Message {
public:
	Message(AdbaseConfig* configure, Storage* storage, Metrics* metrics);
	~Message();
	void start();
	void stop();
	void reload();
	void call(int i);
	MessageToLua getMessage();
    int rollback(std::list<std::string> ids);
	bool push(MessageItem& item);
    bool queueCheck();
    static void deleteThread(std::thread *t);

private:
    mutable std::mutex _mut;
    std::condition_variable _dataCond;
	typedef std::unique_ptr<std::thread, decltype(&Message::deleteThread)> ThreadPtr;
    typedef std::vector<ThreadPtr> ThreadPool;
    ThreadPool Threads;
    int _threadNumber;

	AdbaseConfig *_configure;
	Storage* _storage;
	Metrics* _metrics;
    bool _isRunning;
	std::unordered_map<int, MessageQueue*> _queues;
    std::shared_ptr<adbase::AsyncLogging> _logger;
    std::unordered_map<std::string, adbase::metrics::Meters*> _errorMessageMeter;
    adbase::metrics::Timers* _luaProcessTimer;

	void callMessage();
	void initLua();
	void addLuaMessage(MessageItem& item);
	const std::string convertKey(MessageItem& item);
    void serialize(adbase::Buffer& buffer, MessageItem& item);
    void saveMessage(MessageItem& item);
};
}
#endif
