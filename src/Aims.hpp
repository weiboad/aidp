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
	void stop();
	void reload();
	void pause();
	void resume();

private:
	/// 传输上下文指针
	AimsContext* _context; 
	AdbaseConfig* _configure;
    bool _isResume = true;
    bool _isPause = true;

	void init();
	void initKafkaConsumer();
	aims::kafka::ConsumerOut* _kafkaConsumerCallbackOut = nullptr;
    adbase::kafka::ConsumerBatch* _kafka;
    enum runState {
        STOP = 0,
        RUNNING = 1,
        PAUSE = 2,
    };
    runState _state = STOP;
};

#endif
