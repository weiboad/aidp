#ifndef AIDP_HTTP_INDEX_HPP_
#define AIDP_HTTP_INDEX_HPP_

#include "HttpInterface.hpp"

namespace adserver {
namespace http {
class Index : HttpInterface {
public:
	Index(AdServerContext* context);
	void registerLocation(adbase::http::Server* http);
	void index(adbase::http::Request* request, adbase::http::Response* response, void*);
};
}
}

#endif