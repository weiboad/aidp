#ifndef AIDP_APP_HPP_
#define AIDP_APP_HPP_

#include <adbase/Config.hpp>
#include <adbase/Lua.hpp>
#include "AdbaseConfig.hpp"
#include "App/Message.hpp"

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
	app::Message* _message = nullptr;
	adbase::lua::Engine* _messageEngine = nullptr;
	mutable std::mutex _mut;
	void bindLuaMessage();
};

#endif
