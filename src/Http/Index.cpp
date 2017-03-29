#include "Index.hpp"

namespace adserver {
namespace http {
// {{{ Index::Index()

Index::Index(AdServerContext* context) :
	HttpInterface(context) {
}

// }}}
// {{{ void Index::registerLocation()

void Index::registerLocation(adbase::http::Server* http) {
	ADSERVER_HTTP_ADD_API(http, Index, index)
}

// }}}
// {{{ void Index::index()

void Index::index(adbase::http::Request* request, adbase::http::Response* response, void*) {
	(void)request;
	responseJson(response, "{\"msg\": \"hello adinf\"}", 0, "");
}

// }}}
}
}