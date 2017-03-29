#include "ConsumerOut.hpp"

namespace aims {
namespace kafka {
// {{{	ConsumerOut::ConsumerOut()

ConsumerOut::ConsumerOut(AimsContext* context) :
	_context(context) {
}

// }}}
// {{{ ConsumerOut::~ConsumerOut()

ConsumerOut::~ConsumerOut() {
}

// }}}
// {{{ void ConsumerOut::pull()

bool ConsumerOut::pull(const std::string& topicName, int partId, uint64_t offset, const adbase::Buffer& data) {
	(void)topicName;
	(void)partId;
	(void)offset;
	LOG_INFO << std::string(data.peek(), data.readableBytes());
	return true;
}

// }}}
// {{{ void ConsumerOut::stat()

void ConsumerOut::stat(adbase::kafka::Consumer* consumer, const std::string& stats) {
    LOG_INFO << "Stats:" << stats.substr(0, 1024);
    LOG_INFO << consumer->getTopicName();
}

// }}}
}
}