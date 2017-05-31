#include "Server.hpp"
#include "Version.hpp"

namespace adserver {
namespace http {
// {{{ Server::Server()

Server::Server(AdServerContext* context) :
	HttpInterface(context) {
}

// }}}
// {{{ void Server::registerLocation()

void Server::registerLocation(adbase::http::Server* http) {
	ADSERVER_HTTP_ADD_API(http, Server, status)
	ADSERVER_HTTP_ADD_API(http, Server, metrics)
}

// }}}
// {{{ void Server::status()

void Server::status(adbase::http::Request* request, adbase::http::Response* response, void*) {
	(void)request;
	std::string result;
	// 如下 json 手动拼接为了减少对 json 库的依赖，在实际项目中推荐用 rapidjson
	std::unordered_map<std::string, std::string> procs = adbase::procStats();
	procs["version"]    = VERSION;
	procs["git_sha1"]   = GIT_SHA1;
	procs["git_dirty"]  = GIT_DIRTY;
	procs["build_id"]   = BUILD_ID;
	procs["build_type"] = BUILD_TYPE;
	std::string system = "{";
	for (auto &t : procs) {
		system +="\"" + t.first + "\":\"" + t.second + "\",";
	}
	system = adbase::rightTrim(system, ",");

	// Metrics
	std::unordered_map<std::string, int64_t> gauges;
	std::unordered_map<std::string, int64_t> counters;
	std::unordered_map<std::string, adbase::metrics::MeterItem> meters;
	std::unordered_map<std::string, adbase::metrics::HistogramsItem> histograms;
	std::unordered_map<std::string, adbase::metrics::TimersItem> timers;
	if (_context->metrics != nullptr) {
		gauges = _context->metrics->getGauges();
		counters = _context->metrics->getCounter();
		histograms = _context->metrics->getHistograms();
		meters = _context->metrics->getMeters();
		timers = _context->metrics->getTimers();
	}

	std::unordered_map<std::string, std::string> modulesItems;
	modulesItems["system"] = system;
	for (auto &t : gauges) {
		adbase::metrics::MetricName name = adbase::metrics::Metrics::getMetricName(t.first);
		if (name.moduleName == "self") {
			continue;
		}
		if (modulesItems.find(name.moduleName) == modulesItems.end()) {
			modulesItems[name.moduleName] = "{";
		}

		modulesItems[name.moduleName] += "\"" + name.metricName + "\":" + std::to_string(t.second) + ",";
	}
	for (auto &t : counters) {
		adbase::metrics::MetricName name = adbase::metrics::Metrics::getMetricName(t.first);
		if (name.moduleName == "self") {
			continue;
		}
		if (modulesItems.find(name.moduleName) == modulesItems.end()) {
			modulesItems[name.moduleName] = "{";
		}

		modulesItems[name.moduleName] += "\"" + name.metricName + "\":" + std::to_string(t.second) + ",";
	}
	for (auto &t : meters) {
		adbase::metrics::MetricName name = adbase::metrics::Metrics::getMetricName(t.first);
		if (name.moduleName == "self") {
			continue;
		}
		if (modulesItems.find(name.moduleName) == modulesItems.end()) {
			modulesItems[name.moduleName] = "{";
		}

		std::string meterItem = "{\"count\":" + std::to_string(t.second.count) +
								",\"meanRate\":" + std::to_string(t.second.meanRate) +
								",\"min1Rate\":" + std::to_string(t.second.min1Rate) +
								",\"min5Rate\":" + std::to_string(t.second.min5Rate) +
								",\"min15Rate\":" + std::to_string(t.second.min15Rate) + "}";
					
		modulesItems[name.moduleName] += "\"" + name.metricName + "\":" + meterItem + ",";
	}
	for (auto &t : histograms) {
		adbase::metrics::MetricName name = adbase::metrics::Metrics::getMetricName(t.first);
		if (name.moduleName == "self") {
			continue;
		}
		if (modulesItems.find(name.moduleName) == modulesItems.end()) {
			modulesItems[name.moduleName] = "{";
		}

		std::string hisItem = "{\"min\":" + std::to_string(t.second.min) +
								",\"max\":" + std::to_string(t.second.max) +
								",\"mean\":" + std::to_string(t.second.mean) +
								",\"stddev\":" + std::to_string(t.second.stddev) +
								",\"median\":" + std::to_string(t.second.median) +
								",\"percent75\":" + std::to_string(t.second.percent75) +
								",\"percent95\":" + std::to_string(t.second.percent95) +
								",\"percent98\":" + std::to_string(t.second.percent98) +
								",\"percent99\":" + std::to_string(t.second.percent99) +
								",\"percent999\":" + std::to_string(t.second.percent999) + "}";
					
		modulesItems[name.moduleName] += "\"" + name.metricName + "\":" + hisItem + ",";
	}
	for (auto &t : timers) {
		adbase::metrics::MetricName name = adbase::metrics::Metrics::getMetricName(t.first);
		if (modulesItems.find(name.moduleName) == modulesItems.end()) {
			modulesItems[name.moduleName] = "{";
		}

		std::string timerItem = "{\"count\":" + std::to_string(t.second.meter.count) +
								",\"meanRate\":" + std::to_string(t.second.meter.meanRate) +
								",\"min1Rate\":" + std::to_string(t.second.meter.min1Rate) +
								",\"min5Rate\":" + std::to_string(t.second.meter.min5Rate) +
								",\"min15Rate\":" + std::to_string(t.second.meter.min15Rate) +
								",\"min\":" + std::to_string(t.second.histogram.min) +
								",\"max\":" + std::to_string(t.second.histogram.max) +
								",\"mean\":" + std::to_string(t.second.histogram.mean) +
								",\"stddev\":" + std::to_string(t.second.histogram.stddev) +
								",\"median\":" + std::to_string(t.second.histogram.median) +
								",\"percent75\":" + std::to_string(t.second.histogram.percent75) +
								",\"percent95\":" + std::to_string(t.second.histogram.percent95) +
								",\"percent98\":" + std::to_string(t.second.histogram.percent98) +
								",\"percent99\":" + std::to_string(t.second.histogram.percent99) +
								",\"percent999\":" + std::to_string(t.second.histogram.percent999) + "}";
					
		modulesItems[name.moduleName] += "\"" + name.metricName + "\":" + timerItem + ",";
	}

	result = "{";
	for (auto &t : modulesItems) {
		result += "\"" + t.first + "\":" + adbase::rightTrim(t.second, ",") + "},";  
	}
	result = adbase::rightTrim(result, ",");
	result += "}";

	responseJson(response, result, 0, "");
}

// }}}
// {{{ void Server::metrics()

void Server::metrics(adbase::http::Request* request, adbase::http::Response* response, void*) {
	(void)request;
	std::string result;
    std::string tagValue = request->getQuery("tags"); 
	std::unordered_map<std::string, std::string> tags;
    if (!tagValue.empty()) {
        std::vector<std::string> tagStrs = adbase::explode(tagValue, '|', true);
        for (auto &t: tagStrs) {
            std::vector<std::string> tagKV = adbase::explode(t, ':', true); 
            if (tagKV.size() == 2) {
                tags[tagKV[0]] = tagKV[1];
            }
        }
    }

	// 如下 json 手动拼接为了减少对 json 库的依赖，在实际项目中推荐用 rapidjson
	std::unordered_map<std::string, std::string> procs = adbase::procStats();
    tags["name"] = adbase::trim(procs["name"], "()");


    int count = 0;
    std::string serverAddress = adbase::replace(":", "_", request->getServerAddress(), count);
    serverAddress = adbase::replace(".", "_", serverAddress, count);
    tags["service"] = serverAddress;

	for (auto &t : procs) {
        std::string key = adbase::replace(".", "_", t.first, count);
        tags["metric_type"] = "gauges";
        tags["metric_meta"] = key;
        result += formatMetric(tags["metric_type"] + "_" + serverAddress, 1, tags);
        tags["metric_meta"] = "adbase_value";
        result += formatMetric(key, toUint64(t.second), tags);
	}

	// Metrics
	std::unordered_map<std::string, int64_t> gauges;
	std::unordered_map<std::string, int64_t> counters;
	std::unordered_map<std::string, adbase::metrics::MeterItem> meters;
	std::unordered_map<std::string, adbase::metrics::HistogramsItem> histograms;
	std::unordered_map<std::string, adbase::metrics::TimersItem> timers;
	if (_context->metrics != nullptr) {
		gauges = _context->metrics->getGauges();
		counters = _context->metrics->getCounter();
		histograms = _context->metrics->getHistograms();
		meters = _context->metrics->getMeters();
		timers = _context->metrics->getTimers();
	}

	std::unordered_map<std::string, std::string> modulesItems;
	for (auto &t : gauges) {
		adbase::metrics::MetricName name = adbase::metrics::Metrics::getMetricName(t.first);
		if (name.moduleName == "self") {
			continue;
		}

        std::string key = name.moduleName + "_" + name.metricName;
        key = adbase::replace(".", "_", key, count);
        tags["metric_type"] = "gauges";
        tags["metric_meta"] = key;
        result += formatMetric(tags["metric_type"] + "_" + serverAddress, 1, tags);
        tags["metric_meta"] = "adbase_value";
        result += formatMetric(key, t.second, tags);
	}
	for (auto &t : counters) {
		adbase::metrics::MetricName name = adbase::metrics::Metrics::getMetricName(t.first);
		if (name.moduleName == "self") {
			continue;
		}
        std::string key = name.moduleName + "_" + name.metricName;
        key = adbase::replace(".", "_", key, count);
        tags["metric_type"] = "counters";
        tags["metric_meta"] = key;
        result += formatMetric(tags["metric_type"] + "_" + serverAddress, 1, tags);
        tags["metric_meta"] = "adbase_value";
        result += formatMetric(key, t.second, tags);
	}

    tags["metric_type"] = "meters";
	for (auto &t : meters) {
		adbase::metrics::MetricName name = adbase::metrics::Metrics::getMetricName(t.first);
		if (name.moduleName == "self") {
			continue;
		}
        std::string key = name.moduleName + "_" + name.metricName;
        key = adbase::replace(".", "_", key, count);
        tags["metric_meta"] = key;
        result += formatMetric(tags["metric_type"] + "_" + serverAddress, 1, tags);
        tags["metric_meta"] = "adbase_value";
        result += formatMetric(key, t.second.count, tags);
        result += formatMetric(key + "_mean_rate", toUint64(t.second.meanRate), tags);
        result += formatMetric(key + "_min1_rate", toUint64(t.second.min1Rate), tags);
        result += formatMetric(key + "_min5_rate", toUint64(t.second.min5Rate), tags);
        result += formatMetric(key + "_min15_rate", toUint64(t.second.min15Rate), tags);
	}

    tags["metric_type"] = "histograms";
	for (auto &t : histograms) {
		adbase::metrics::MetricName name = adbase::metrics::Metrics::getMetricName(t.first);
		if (name.moduleName == "self") {
			continue;
		}

        std::string key = name.moduleName + "_" + name.metricName;
        key = adbase::replace(".", "_", key, count);
        tags["metric_meta"] = key;
        result += formatMetric(tags["metric_type"] + "_" + serverAddress, 1, tags);
        tags["metric_meta"] = "adbase_value";
        result += formatMetric(key, 1, tags);
        result += formatMetric(key + "_min", toUint64(t.second.min), tags);
        result += formatMetric(key + "_max", toUint64(t.second.max), tags);
        result += formatMetric(key + "_mean", toUint64(t.second.mean), tags);
        result += formatMetric(key + "_stddev", toUint64(t.second.stddev), tags);
        result += formatMetric(key + "_median", toUint64(t.second.median), tags);
        result += formatMetric(key + "_percent75", toUint64(t.second.percent75), tags);
        result += formatMetric(key + "_percent95", toUint64(t.second.percent95), tags);
        result += formatMetric(key + "_percent98", toUint64(t.second.percent98), tags);
        result += formatMetric(key + "_percent99", toUint64(t.second.percent99), tags);
        result += formatMetric(key + "_percent999", toUint64(t.second.percent999), tags);
	}

    tags["metric_type"] = "timers";
	for (auto &t : timers) {
		adbase::metrics::MetricName name = adbase::metrics::Metrics::getMetricName(t.first);

        std::string key = name.moduleName + "_" + name.metricName;
        key = adbase::replace(".", "_", key, count);
        tags["metric_meta"] = key;
        result += formatMetric(tags["metric_type"] + "_" + serverAddress, 1, tags);
        tags["metric_meta"] = "adbase_value";
        result += formatMetric(key, t.second.meter.count, tags);
        result += formatMetric(key + "_mean_rate", toUint64(t.second.meter.meanRate), tags);
        result += formatMetric(key + "_min1_rate", toUint64(t.second.meter.min1Rate), tags);
        result += formatMetric(key + "_min5_rate", toUint64(t.second.meter.min5Rate), tags);
        result += formatMetric(key + "_min15_rate", toUint64(t.second.meter.min15Rate), tags);
        result += formatMetric(key + "_min", toUint64(t.second.histogram.min), tags);
        result += formatMetric(key + "_max", toUint64(t.second.histogram.max), tags);
        result += formatMetric(key + "_mean", toUint64(t.second.histogram.mean), tags);
        result += formatMetric(key + "_stddev", toUint64(t.second.histogram.stddev), tags);
        result += formatMetric(key + "_median", toUint64(t.second.histogram.median), tags);
        result += formatMetric(key + "_percent75", toUint64(t.second.histogram.percent75), tags);
        result += formatMetric(key + "_percent95", toUint64(t.second.histogram.percent95), tags);
        result += formatMetric(key + "_percent98", toUint64(t.second.histogram.percent98), tags);
        result += formatMetric(key + "_percent99", toUint64(t.second.histogram.percent99), tags);
        result += formatMetric(key + "_percent999", toUint64(t.second.histogram.percent999), tags);
	}

    responseHeader(response);
    response->setContent(result);
    response->sendReply();
}

// }}}
// {{{ const std::string Server::formatMetric()

const std::string Server::formatMetric(std::string key, uint64_t value, std::unordered_map<std::string, std::string> tags) {
    std::string result = "adbase_"; 

    int count = 0;
    std::string keyFormat = adbase::replace(".", "_", key, count);
    result += keyFormat;
    if (!tags.empty()) {
        result += "{";
        for (auto &t : tags) {
            result += t.first; 
            result += "=\""; 
            result += t.second; 
            result += "\","; 
        }
        result = adbase::rightTrim(result, ",");
        result += "} ";
    }

    result += "  ";
    result += std::to_string(value) + "\n";
    return result;
}

// }}}
// {{{ uint64_t Server::toUint64()

uint64_t Server::toUint64(std::string value) {
    errno = 0;
    uint64_t result = static_cast<uint64_t>(strtoull(value.c_str(), nullptr, 10));
    if (errno != 0) {
        return 0;
    }
    return result;
}

// }}}
// {{{ uint64_t Server::toUint64()

uint64_t Server::toUint64(double value) {
    return static_cast<uint64_t>(value * 10000);
}

// }}}
}
}
