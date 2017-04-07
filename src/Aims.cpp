#include "Aims.hpp"

// {{{ macros

#define STOP_KAFKA_CONSUMER(name) do {\
} while(0)

#define INIT_KAFKA_PRODUCER(name) do {\
	_kafkaProducerCallback##name = new aims::kafka::Producer##name(_context);\
	_kafkaProducer##name = new adbase::kafka::Producer(_configure->brokerListProducer##name,\
						   _configure->queueLength##name, _configure->debug##name);\
	_kafkaProducer##name->setSendHandler(std::bind(&aims::kafka::Producer##name::send,\
											   _kafkaProducerCallback##name,\
											   std::placeholders::_1, std::placeholders::_2,\
											   std::placeholders::_3, std::placeholders::_4));\
	_kafkaProducer##name->setAckHandler(std::bind(&aims::kafka::Producer##name::ackCallback, \
													 _kafkaProducerCallback##name, std::placeholders::_1));\
	_kafkaProducer##name->setErrorHandler(std::bind(&aims::kafka::Producer##name::errorCallback, \
													 _kafkaProducerCallback##name, std::placeholders::_1));\
} while(0)
#define START_KAFKA_PRODUCER(name) do {\
	_kafkaProducer##name->start();\
} while(0)
#define STOP_KAFKA_PRODUCER(name) do {\
	if (_kafkaProducer##name != nullptr) {\
		_kafkaProducer##name->stop();\
	}\
	if (_kafkaProducerCallback##name != nullptr) {\
		delete _kafkaProducerCallback##name;\
		_kafkaProducerCallback##name = nullptr;\
	}\
} while(0)

// }}}
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

	for(auto &t : _kafkas) {
		t.second->start();	
	}
}

// }}}
// {{{ void Aims::init()

void Aims::init() {
	initKafkaConsumer();
}

// }}}
// {{{ void Aims::stop()

void Aims::stop() {
	for (auto &t : _kafkas) {
		if (t.second != nullptr) {
			t.second->stop();
		}
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
	for(auto &t : topicNames) {
		adbase::kafka::Consumer* consumer = new adbase::kafka::Consumer(t, _configure->groupIdOut, 
											_configure->brokerListConsumerOut);	
		consumer->setMessageHandler(std::bind(&aims::kafka::ConsumerOut::pull,
												_kafkaConsumerCallbackOut,
												std::placeholders::_1, std::placeholders::_2,
												std::placeholders::_3, std::placeholders::_4));
		consumer->setStatCallback(std::bind(&aims::kafka::ConsumerOut::stat,
											_kafkaConsumerCallbackOut,
											std::placeholders::_1, std::placeholders::_2));
		consumer->setKafkaDebug(_configure->kafkaDebugOut);
		consumer->setOffsetStorePath(_configure->offsetPathOut);
		consumer->setKafkaStatInterval(_configure->statIntervalOut);
		if (_configure->isNewConsumerOut) {
			consumer->setIsNewConsumer(true);
			consumer->setOffsetStoreMethod("broker");
		}
		_kafkas[t] = consumer;
	}
}

// }}}
