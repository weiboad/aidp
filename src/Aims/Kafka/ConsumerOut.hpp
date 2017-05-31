#ifndef AIDP_AIMS_KAFKA_CONSUMER_OUTHPP_
#define AIDP_AIMS_KAFKA_CONSUMER_OUTHPP_

#include <adbase/Kafka.hpp>
#include <adbase/Logging.hpp>
#include "AdbaseConfig.hpp"

namespace aims {
namespace kafka {
class ConsumerOut {
public:
	ConsumerOut(AimsContext* context);
	~ConsumerOut();
	bool pull(const std::string& topicName, int partId, uint64_t offset, const adbase::Buffer& data);
	void stat(adbase::kafka::ConsumerBatch* consumer, const std::string& stats);
private:
	AimsContext* _context;
};

}
}
#endif
