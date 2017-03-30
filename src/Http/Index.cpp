#include "Index.hpp"
#include <adbase/Logging.hpp>

namespace adserver {
namespace http {

thread_local std::weak_ptr<adbase::http::Request> tmpRequest;
thread_local std::weak_ptr<adbase::http::Response> tmpResponse;

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
	thread_local adbase::lua::Engine* httpLuaEngine = nullptr;
	if (httpLuaEngine == nullptr) {
		httpLuaEngine = new adbase::lua::Engine();
		httpLuaEngine->init();
		httpLuaEngine->clearLoaded();
		httpLuaEngine->addSearchPath(_context->config->luaScriptPath, true);
		bindLuaClass(httpLuaEngine);
		LOG_INFO << "Http Lua engine init....";
	}

	auto sharedRequest = std::shared_ptr<adbase::http::Request>(request, [](adbase::http::Request*) { LOG_INFO << "delete Request";});
	auto sharedResponse = std::shared_ptr<adbase::http::Response>(response, [](adbase::http::Response*) { LOG_INFO << "delete Response";});
	tmpRequest = sharedRequest;
	tmpResponse = sharedResponse;
	responseHeader(response);
	response->setContent("");
	//httpLuaEngine->runFile(_context->config->httpScriptName.c_str());
	responseJson(response, "{\"msg\": \"hello xxxx adinf\"}", 0, "");
	sharedResponse.reset();
	sharedRequest.reset();
	response->sendReply();
}

// }}}
// {{{ void Index::getRequest()

std::weak_ptr<adbase::http::Request>& Index::getRequest() {
	LOG_INFO << "return request....";
	return tmpRequest;
}

// }}}
// {{{ void Index::getResponse()

std::weak_ptr<adbase::http::Response>& Index::getResponse() {
	return tmpResponse;
}

// }}}
// {{{ void Index::bindLuaClass()

void Index::bindLuaClass(adbase::lua::Engine* engine) {
	adbase::lua::BindingClass<adbase::http::Request> request("request", "aidp.http", engine->getLuaState());
	typedef std::function<std::weak_ptr<adbase::http::Request>()> GetRequest;
	GetRequest requestFn = std::bind(&adserver::http::Index::getRequest, this);
	request.addMethod("request", requestFn);
	request.addMethod("getUri", &adbase::http::Request::getUri);

	// bind response
	adbase::lua::BindingClass<adbase::http::Response> response("response", "aidp.http", engine->getLuaState());
	typedef std::function<std::weak_ptr<adbase::http::Response>()> GetResponse;
	GetResponse responseFn = std::bind(&adserver::http::Index::getResponse, this);
	response.addMethod("response", responseFn); // 构造函数
	response.addMethod("setContent", &adbase::http::Response::setContent);
}

// }}}
}
}
