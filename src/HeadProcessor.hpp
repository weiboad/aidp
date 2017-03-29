#ifndef AIDP_HEAD_PROCESSOR_HPP_
#define AIDP_HEAD_PROCESSOR_HPP_

#include <adbase/Utility.hpp>
#include <adbase/Logging.hpp>
#include <adbase/Head.hpp>
#include "AdbaseConfig.hpp"

class HeadProcessor {
public:
	HeadProcessor(AdServerContext* context);
	~HeadProcessor();
    adbase::head::ProtocolBinaryResponseStatus readHandler(adbase::head::ProtocolBinaryDataType datatype,
            const void *data,
            ssize_t datalen,
			adbase::head::ProtocolBinaryDataType* resDataType,
			adbase::Buffer* responseData);

private:
	/// 传输上下文指针
	AdServerContext* _context; 
	AdbaseConfig* _configure;
};

#endif