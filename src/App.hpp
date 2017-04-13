#ifndef AIDP_APP_HPP_
#define AIDP_APP_HPP_

#include <adbase/Config.hpp>
#include <adbase/Lua.hpp>
#include "AdbaseConfig.hpp"
#include "App/Message.hpp"
#include "App/Storage.hpp"

class App {
public:
	App(AdbaseConfig* config);
	~App();
	void reload();
	void run();
	void stop();
	void setAdServerContext(AdServerContext* context);
	void setTimerContext(TimerContext* context);
	void setAimsContext(AimsContext* context);
	void loadConfig(adbase::IniConfig& config);
	uint64_t getSeqId();

private:
	AdbaseConfig* _configure;
	std::shared_ptr<app::Storage> _storage;
	std::shared_ptr<app::Message> _message;
	mutable std::mutex _mut;
	void bindLuaMessage();
};

#endif
