/// 该程序自动生成，禁止修改
#ifndef AIDP_BOOTSTRAP_HPP_
#define AIDP_BOOTSTRAP_HPP_

#include <signal.h>
#include <thread>
#include <adbase/Utility.hpp>
#include <adbase/Logging.hpp>
#include <adbase/Net.hpp>
#include "AdbaseConfig.hpp"

class App;
class Timer;
class AdServer;
class Aims;
class BootStrap {
public:
	BootStrap();
	~BootStrap();
	void init(int argc, char **argv);
	void run();
	void reload();
	void stop(const int sig);
	AdbaseConfig* getConfig();

private:
    std::shared_ptr<AdbaseConfig> _configure;
	mutable std::mutex _mut;
    std::shared_ptr<adbase::AsyncLogging> _asnclog;
	std::string _configFile;
    std::shared_ptr<AdServer> _adServer;
    std::shared_ptr<adbase::EventLoop> _loop;
    std::shared_ptr<Aims> _aims;
    std::shared_ptr<Timer> _timer;
    std::unique_ptr<App> _app;

	void daemonInit();
	void asyncLogger(const char* msg, int len);
	void loggerInit(const adbase::TimeZone& tz);
	void setLoggerLevel();
	void loadConfig();
	void parseOption(int argc, char **argv);
	void usage();
	void printVersion();
};

#endif
