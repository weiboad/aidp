#include <adbase/Utility.hpp>
#include <adbase/Logging.hpp>
#include <adbase/Lua.hpp>
#include "App.hpp"

//{{{ macros

#define LOAD_TIMER_CONFIG(name) do {\
	_configure->interval##name = config.getOptionUint32("timer", "interval"#name);\
} while(0)

#define LOAD_KAFKA_CONSUMER_CONFIG(name, sectionName) do {\
        _configure->topicNameConsumer##name    = config.getOption("kafkac_"#sectionName, "topicName"#name);\
        _configure->groupId##name      = config.getOption("kafkac_"#sectionName, "groupId"#name);\
        _configure->brokerListConsumer##name   = config.getOption("kafkac_"#sectionName, "brokerList"#name);\
        _configure->kafkaDebug##name   = config.getOption("kafkac_"#sectionName, "kafkaDebug"#name);\
        _configure->statInterval##name = config.getOption("kafkac_"#sectionName, "statInterval"#name);\
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
	_storage = std::shared_ptr<app::Storage>(new app::Storage(_configure));
	_storage->init();

	_metrics = std::shared_ptr<app::Metrics>(new app::Metrics());

	_message = std::shared_ptr<app::Message>(new app::Message(_configure, _storage.get(), _metrics.get()));
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
	_message->stop();
}

// }}}
// {{{ void App::setAdServerContext()

void App::setAdServerContext(AdServerContext* context) {
	context->app = this;
	context->storage = _storage.get();
	context->appMetrics = _metrics.get();
}

// }}}
// {{{ void App::setAimsContext()

void App::setAimsContext(AimsContext* context) {
	context->app = this;
	context->message = _message.get();
	context->storage = _storage.get();
	context->appMetrics = _metrics.get();
}

// }}}
// {{{ void App::setTimerContext()

void App::setTimerContext(TimerContext* context) {
	context->app = this;
	context->storage = _storage.get();
	context->appMetrics = _metrics.get();
}

// }}}
//{{{ void App::loadConfig()

void App::loadConfig(adbase::IniConfig& config) {
    _configure->luaDebug = config.getOptionBool("lua", "debug");
    _configure->luaScriptPath = config.getOption("lua", "scriptPath");
    _configure->consumerScriptName  = config.getOption("consumer", "scriptName");
    _configure->consumerBatchNumber = config.getOptionUint32("consumer", "batchNumber");
    _configure->consumerThreadNumber = config.getOptionUint32("consumer", "threadNumber");
    _configure->consumerMaxNumber = config.getOptionUint32("consumer", "maxNumber");
    _configure->messageSwp = config.getOption("consumer", "messageSwp");
    _configure->httpScriptName  = config.getOption("http", "scriptName");
	
    LOAD_KAFKA_CONSUMER_CONFIG(Out, out);
	LOAD_TIMER_CONFIG(ClearStorage);
}

//}}}
//{{{ uint64_t App::getSeqId()

uint64_t App::getSeqId() {
	std::lock_guard<std::mutex> lk(_mut);
	adbase::Sequence seq;
	return seq.getSeqId(static_cast<uint16_t>(_configure->macid), static_cast<uint16_t>(_configure->appid));
}

//}}}
