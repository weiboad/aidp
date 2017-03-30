#include <adbase/Utility.hpp>
#include <adbase/Logging.hpp>
#include <adbase/Lua.hpp>
#include "App.hpp"

//{{{ macros

#define LOAD_KAFKA_CONSUMER_CONFIG(name, sectionName) do {\
    _configure->isNewConsumer##name = config.getOptionBool("kafkac_"#sectionName, "isNewConsumer"#name);\
    _configure->topicNameConsumer##name    = config.getOption("kafkac_"#sectionName, "topicName"#name);\
    _configure->groupId##name      = config.getOption("kafkac_"#sectionName, "groupId"#name);\
    _configure->brokerListConsumer##name   = config.getOption("kafkac_"#sectionName, "brokerList"#name);\
    _configure->kafkaDebug##name   = config.getOption("kafkac_"#sectionName, "kafkaDebug"#name);\
    _configure->offsetPath##name   = config.getOption("kafkac_"#sectionName, "offsetPath"#name);\
    _configure->statInterval##name = config.getOption("kafkac_"#sectionName, "statInterval"#name);\
} while(0)

#define LOAD_KAFKA_PRODUCER_CONFIG(name, sectionName) do {\
    _configure->topicNameProducer##name    = config.getOption("kafkap_"#sectionName, "topicName"#name);\
    _configure->brokerListProducer##name   = config.getOption("kafkap_"#sectionName, "brokerList"#name);\
    _configure->debug##name        = config.getOption("kafkap_"#sectionName, "debug"#name);\
    _configure->queueLength##name  = config.getOptionUint32("kafkap_"#sectionName, "queueLength"#name);\
} while(0)

#define LOAD_TIMER_CONFIG(name) do {\
	_configure->interval##name = config.getOptionUint32("timer", "interval"#name);\
} while(0)

//}}}
// {{{ App::App()

App::App(AdbaseConfig* config) :
	_configure(config) {
}

// }}}
// {{{ App::~App()

App::~App() {
}

// }}}
// {{{ void App::run()

void App::run() {
	_messageEngine = new adbase::lua::Engine();
	_messageEngine->init();
	_messageEngine->clearLoaded();
	_messageEngine->addSearchPath(_configure->luaScriptPath, true);

	_message = new app::Message(_configure, _messageEngine);
	// 绑定 class
	bindLuaMessage();
	_message->loadMessage();
	_message->start();
}

// }}}
// {{{ void App::reload()

void App::reload() {
	// 重新加载 message
	_message->reload();
}

// }}}
// {{{ void App::stop()

void App::stop() {
	if (_message != nullptr) {
		_message->stop();
		delete _message;
		_message = nullptr;
	}
	if (_messageEngine != nullptr) {
		delete _messageEngine;
		_messageEngine = nullptr;
	}
}

// }}}
// {{{ void App::setAdServerContext()

void App::setAdServerContext(AdServerContext* context) {
	context->app = this;
}

// }}}
// {{{ void App::setAimsContext()

void App::setAimsContext(AimsContext* context) {
	context->app = this;
	context->message = _message;
}

// }}}
// {{{ void App::setTimerContext()

void App::setTimerContext(TimerContext* context) {
	context->app = this;
}

// }}}
//{{{ void App::loadConfig()

void App::loadConfig(adbase::IniConfig& config) {
	LOAD_KAFKA_CONSUMER_CONFIG(Out, out);

    _configure->luaDebug = config.getOptionBool("lua", "debug");
    _configure->luaScriptPath = config.getOption("lua", "scriptPath");
    _configure->consumerScriptName  = config.getOption("consumer", "scriptName");
    _configure->consumerBatchNumber = config.getOptionUint32("consumer", "batchNumber");
    _configure->messageSwp = config.getOption("consumer", "messageSwp");
    _configure->httpScriptName  = config.getOption("http", "scriptName");
	
	LOAD_TIMER_CONFIG(Noop);
}

//}}}
//{{{ void App::bindLuaMessage()

void App::bindLuaMessage() {
	adbase::lua::BindingClass<app::Message> clazz("message", "aidp", _messageEngine->getLuaState());
	typedef std::function<std::list<std::pair<std::string, std::string>>()> GetMessageFn;
	GetMessageFn getMessageFn = std::bind(&app::Message::getMessage, _message);
	clazz.addMethod("get", getMessageFn);
}

//}}}
//{{{ uint64_t App::getSeqId()

uint64_t App::getSeqId() {
	std::lock_guard<std::mutex> lk(_mut);
	adbase::Sequence seq;
	return seq.getSeqId(static_cast<uint16_t>(_configure->macid), static_cast<uint16_t>(_configure->appid));
}

//}}}
