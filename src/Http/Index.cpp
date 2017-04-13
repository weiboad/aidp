#include "Index.hpp"
#include "App/Storage.hpp"
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
	httpLuaEngine->runFile(_context->config->httpScriptName.c_str());
	//responseJson(response, "{\"msg\": \"hello xxxx adinf\"}", 0, "");
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
	adbase::lua::BindingNamespace requestCs = request.getOwnerNamespace();
	typedef std::function<std::weak_ptr<adbase::http::Request>()> GetRequest;
	GetRequest requestFn = std::bind(&adserver::http::Index::getRequest, this);
	requestCs.addMethod("request", requestFn);

	request.addMethod("get_uri", &adbase::http::Request::getUri);
	request.addMethod("get_remote_address", &adbase::http::Request::getRemoteAddress);
	request.addMethod("get_post_data", &adbase::http::Request::getPostData);
	request.addMethod("get_post", &adbase::http::Request::getPost);
	request.addMethod("get_query", &adbase::http::Request::getQuery);
	request.addMethod("get_header", &adbase::http::Request::getHeader);
	request.addMethod("get_location", &adbase::http::Request::getLocation);
	request.addMethod("get_method", &adbase::http::Request::getMethod);
	request.addConst("METHOD_GET", adbase::http::Request::httpMethod::METHOD_GET);
	request.addConst("METHOD_POST", adbase::http::Request::httpMethod::METHOD_POST);
	request.addConst("METHOD_OTHER", adbase::http::Request::httpMethod::METHOD_OTHER);

	// bind response
	adbase::lua::BindingClass<adbase::http::Response> response("response", "aidp.http", engine->getLuaState());
	adbase::lua::BindingNamespace responseCs = response.getOwnerNamespace();
	typedef std::function<std::weak_ptr<adbase::http::Response>()> GetResponse;
	GetResponse responseFn = std::bind(&adserver::http::Index::getResponse, this);
	responseCs.addMethod("response", responseFn); // 构造函数

	response.addMethod("set_header", &adbase::http::Response::setHeader);
	response.addMethod("add_header", &adbase::http::Response::addHeader);
	response.addMethod("set_content", &adbase::http::Response::setContent);
	response.addMethod("append_content", &adbase::http::Response::appendContent);
	response.addMethod("send_reply", &adbase::http::Response::sendReply);
	response.addMethod("get_code", &adbase::http::Response::getCode);
	response.addMethod("set_body_size", &adbase::http::Response::getBodySize);
	_context->storage->bindClass(engine);
}

// }}}
}
}
