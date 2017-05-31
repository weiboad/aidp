#include "Aims.hpp"

// {{{ Aims::Aims()

Aims::Aims(AimsContext* context) :
	_context(context) {
	_configure = _context->config;
}

// }}}
// {{{ Aims::~Aims()

Aims::~Aims() {
	stop();
}

// }}}
// {{{ void Aims::run()

void Aims::run() {
	// 初始化 server
	init();
    _kafka->start();
}

// }}}
// {{{ void Aims::reload()

void Aims::reload() {
	std::vector<std::string> topicNames = adbase::explode(_configure->topicNameConsumerOut, ',', true);
    _kafka->setTopics(topicNames);
}

// }}}
// {{{ void Aims::init()

void Aims::init() {
	initKafkaConsumer();
}

// }}}
// {{{ void Aims::stop()

void Aims::stop() {
    if (_kafka != nullptr) {
        _kafka->stop();
        delete _kafka;
    }
	if (_kafkaConsumerCallbackOut != nullptr) {
		delete _kafkaConsumerCallbackOut;
		_kafkaConsumerCallbackOut = nullptr;
	}
}

// }}}
// {{{ void Aims::initKafkaConsumer()

void Aims::initKafkaConsumer() {
	_kafkaConsumerCallbackOut = new aims::kafka::ConsumerOut(_context);
	std::vector<std::string> topicNames = adbase::explode(_configure->topicNameConsumerOut, ',', true);
    LOG_INFO << "Topic list:" << _configure->topicNameConsumerOut;

    _kafka = new adbase::kafka::ConsumerBatch(topicNames, _configure->groupIdOut, 
                                            _configure->brokerListConsumerOut);
    _kafka->setMessageHandler(std::bind(&aims::kafka::ConsumerOut::pull,
                _kafkaConsumerCallbackOut,
                std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3, std::placeholders::_4));
    _kafka->setStatCallback(std::bind(&aims::kafka::ConsumerOut::stat,
                _kafkaConsumerCallbackOut,
                std::placeholders::_1, std::placeholders::_2));
	_kafka->setKafkaDebug(_configure->kafkaDebugOut);
	_kafka->setKafkaStatInterval(_configure->statIntervalOut);
}

// }}}
