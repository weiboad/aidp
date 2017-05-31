#include "Metrics.hpp"

namespace app {
thread_local std::unordered_map<std::string, std::shared_ptr<adbase::metrics::Counter>> metricsCounters;
thread_local std::unordered_map<std::string, std::shared_ptr<adbase::metrics::Meters>> metricsMeters;
thread_local std::unordered_map<std::string, std::shared_ptr<adbase::metrics::Histograms>> metricsHistograms;
thread_local std::unordered_map<std::string, std::shared_ptr<adbase::metrics::Timers>> metricsTimers;
thread_local std::shared_ptr<adbase::metrics::Timer> metricsTimer;
// {{{ Metrics::Metrics()

Metrics::Metrics() {
}

// }}}
// {{{ Metrics::~Metrics()

Metrics::~Metrics() {
}

// }}}
// {{{ void Metrics::bindClass()

void Metrics::bindClass(adbase::lua::Engine* engine) {
    // metrics
	adbase::lua::BindingClass<Metrics> metrics("metrics", "aidp", engine->getLuaState());
	adbase::lua::BindingNamespace metricsCs = metrics.getOwnerNamespace();
	typedef std::function<std::weak_ptr<Metrics>()> GetMetrics;
	GetMetrics metricsFn = std::bind(&Metrics::getMetrics, this);
	metricsCs.addMethod("metrics", metricsFn);
	metrics.addMethod("counter", &Metrics::buildCounter);
	metrics.addMethod("meters", &Metrics::buildMeters);
	metrics.addMethod("histograms", &Metrics::buildHistograms);
	metrics.addMethod("timers", &Metrics::buildTimers);
	metrics.addMethod("timer", &Metrics::getTimer);

    // counter
	adbase::lua::BindingClass<adbase::metrics::Counter> counter("counter", "aidp", engine->getLuaState());
    counter.addMethod("module_name", &adbase::metrics::Counter::getModuleName);
    counter.addMethod("metric_name", &adbase::metrics::Counter::getMetricName);
    counter.addMethod("add", &adbase::metrics::Counter::add);
    counter.addMethod("dec", &adbase::metrics::Counter::dec);

    // meters
	adbase::lua::BindingClass<adbase::metrics::Meters> meters("meters", "aidp", engine->getLuaState());
    meters.addMethod("module_name", &adbase::metrics::Meters::getModuleName);
    meters.addMethod("metric_name", &adbase::metrics::Meters::getMetricName);
    meters.addMethod("mark", &adbase::metrics::Meters::mark);
    meters.addMethod("get_count", &adbase::metrics::Meters::getCounter);

    LOG_INFO << "TOP:" << lua_gettop(engine->getLuaState());

    // histograms
	adbase::lua::BindingClass<adbase::metrics::Histograms> histograms("histograms", "aidp", engine->getLuaState());
    histograms.addMethod("module_name", &adbase::metrics::Histograms::getModuleName);
    histograms.addMethod("metric_name", &adbase::metrics::Histograms::getMetricName);
    histograms.addMethod("update", &adbase::metrics::Histograms::update);

    // timers
	adbase::lua::BindingClass<adbase::metrics::Timers> timers("timers", "aidp", engine->getLuaState());
    timers.addMethod("module_name", &adbase::metrics::Timers::getModuleName);
    timers.addMethod("metric_name", &adbase::metrics::Timers::getMetricName);
    timers.addMethod("set_timer", &adbase::metrics::Timers::setTimer);

    // timer
	adbase::lua::BindingClass<adbase::metrics::Timer> timer("timer", "aidp", engine->getLuaState());
    timer.addMethod("start", &adbase::metrics::Timer::start);
    timer.addMethod("stop", &adbase::metrics::Timer::stop);
}

// }}}
// {{{ std::weak_ptr<Metrics> Metrics::getMetrics() 

std::weak_ptr<Metrics> Metrics::getMetrics() {
	return shared_from_this();
}

// }}}
// {{{ std::weak_ptr<adbase::metrics::Timer> Metrics::getTimer() 

std::weak_ptr<adbase::metrics::Timer> Metrics::getTimer() {
    if (metricsTimer == false) {
        metricsTimer =  std::shared_ptr<adbase::metrics::Timer>(new adbase::metrics::Timer()); 
    }
	return metricsTimer;
}

// }}}
// {{{ std::weak_ptr<adbase::metrics::Counter> Metrics::buildCounter() 

std::weak_ptr<adbase::metrics::Counter> Metrics::buildCounter(const std::string& moduleName, const std::string& metricName) {
    adbase::metrics::Counter* counter = adbase::metrics::Metrics::buildCounter(moduleName, metricName);
    auto sharedCounter = std::shared_ptr<adbase::metrics::Counter>(counter, [](adbase::metrics::Counter*) { LOG_INFO << "delete Counter";});
    std::string key = getKey(moduleName, metricName);
    metricsCounters[key] = sharedCounter;
    return metricsCounters[key];
}

// }}}
// {{{ std::weak_ptr<adbase::metrics::Meters> Metrics::buildMeters() 

std::weak_ptr<adbase::metrics::Meters> Metrics::buildMeters(const std::string& moduleName, const std::string& metricName) {
    adbase::metrics::Meters* meters = adbase::metrics::Metrics::buildMeters(moduleName, metricName);
    auto sharedMeters = std::shared_ptr<adbase::metrics::Meters>(meters, [](adbase::metrics::Meters*) { LOG_INFO << "delete meters";});
    std::string key = getKey(moduleName, metricName);
    metricsMeters[key] = sharedMeters;
    return metricsMeters[key];
}

// }}}
// {{{ std::weak_ptr<adbase::metrics::Histograms> Metrics::buildHistograms() 

std::weak_ptr<adbase::metrics::Histograms> Metrics::buildHistograms(const std::string& moduleName, const std::string& metricName, uint32_t interval) {
    adbase::metrics::Histograms* histograms = adbase::metrics::Metrics::buildHistograms(moduleName, metricName, interval);
    auto sharedHistograms = std::shared_ptr<adbase::metrics::Histograms>(histograms, [](adbase::metrics::Histograms*) { LOG_INFO << "delete histograms";});
    std::string key = getKey(moduleName, metricName);
    metricsHistograms[key] = sharedHistograms;
    return metricsHistograms[key];
}

// }}}
// {{{ std::weak_ptr<adbase::metrics::Timers> Metrics::buildHistograms() 

std::weak_ptr<adbase::metrics::Timers> Metrics::buildTimers(const std::string& moduleName, const std::string& metricName, uint32_t interval) {
    adbase::metrics::Timers* timers = adbase::metrics::Metrics::buildTimers(moduleName, metricName, interval);
    auto sharedTimers = std::shared_ptr<adbase::metrics::Timers>(timers, [](adbase::metrics::Timers*) { LOG_INFO << "delete timers";});
    std::string key = getKey(moduleName, metricName);
    metricsTimers[key] = sharedTimers;
    return metricsTimers[key];
}

// }}}
// {{{ const std::string Metrics::getKey()

const std::string Metrics::getKey(const std::string& moduleName, const std::string& metricName) {
    std::string result = moduleName;
    result.append(1, 26);
    result.append(metricName);
    return result;
}

// }}}
}
