#ifndef AIDP_AIMS_HPP_
#define AIDP_AIMS_HPP_

#include <adbase/Utility.hpp>
#include <adbase/Logging.hpp>
#include <adbase/Kafka.hpp>
#include "AdbaseConfig.hpp"
#include "Aims/Kafka/ConsumerOut.hpp"

class Aims {
public:
	Aims(AimsContext* context);
	~Aims();
	void run();
	void reload();

private:
	/// 传输上下文指针
	AimsContext* _context; 
	AdbaseConfig* _configure;

	void init();
	void stop();
	void initKafkaConsumer();
	aims::kafka::ConsumerOut* _kafkaConsumerCallbackOut = nullptr;
    adbase::kafka::ConsumerBatch* _kafka;

};

#endif
