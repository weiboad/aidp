#ifndef AIDP_APP_METRICS_HPP_
#define AIDP_APP_METRICS_HPP_

#include <adbase/Utility.hpp>
#include <adbase/Logging.hpp>
#include <adbase/Metrics.hpp>
#include <adbase/Lua.hpp>
#include <unordered_map>
#include "AdbaseConfig.hpp"

namespace app {
class Metrics : public std::enable_shared_from_this<Metrics> {
public:
	Metrics();
	~Metrics();
	void bindClass(adbase::lua::Engine* engine);
	std::weak_ptr<Metrics> getMetrics();
    std::weak_ptr<adbase::metrics::Counter>  buildCounter(const std::string& moduleName, const std::string& metricName);
    std::weak_ptr<adbase::metrics::Meters>  buildMeters(const std::string& moduleName, const std::string& metricName);
    std::weak_ptr<adbase::metrics::Histograms>  buildHistograms(const std::string& moduleName, const std::string& metricName, uint32_t interval);
    std::weak_ptr<adbase::metrics::Timers>  buildTimers(const std::string& moduleName, const std::string& metricName, uint32_t interval);
    std::weak_ptr<adbase::metrics::Timer>  getTimer();
private:
    const std::string getKey(const std::string& moduleName, const std::string& metricName);
};
}
#endif
