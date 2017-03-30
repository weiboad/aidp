#ifndef AIDP_HTTP_INDEX_HPP_
#define AIDP_HTTP_INDEX_HPP_

#include <adbase/Lua.hpp>
#include "HttpInterface.hpp"

namespace adserver {
namespace http {
class Index : HttpInterface {
public:
	Index(AdServerContext* context);
	void registerLocation(adbase::http::Server* http);
	void index(adbase::http::Request* request, adbase::http::Response* response, void*);
	std::weak_ptr<adbase::http::Request>& getRequest();
	std::weak_ptr<adbase::http::Response>& getResponse();
private:
	void bindLuaClass(adbase::lua::Engine* engine);
};
}
}

#endif
