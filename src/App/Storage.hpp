#ifndef AIDP_APP_STORAGE_HPP_
#define AIDP_APP_STORAGE_HPP_

#include <adbase/Utility.hpp>
#include <adbase/Logging.hpp>
#include <adbase/Metrics.hpp>
#include <adbase/Lua.hpp>
#include <unordered_map>
#include <mutex>
#include "AdbaseConfig.hpp"

namespace app {
class Storage : public std::enable_shared_from_this<Storage> {
public:
	Storage(AdbaseConfig* configure);
	~Storage();
	void init();

	// key - value
	bool set(const std::string key, const std::string value);
	bool set(const std::string key, const std::string value, int ttl);
	int64_t incr(const std::string key, int step);
	int64_t decr(const std::string key, int step);
	const std::string get(const std::string key);
	bool deleteKey(const std::string key);
	bool exists(const std::string key);

	void clear();
	void bindClass(adbase::lua::Engine* engine);
	std::weak_ptr<Storage> getStorage();

private:
	AdbaseConfig *_configure;
	mutable std::mutex _mut;

	std::unordered_map<std::string, int> _incrMap;
	std::unordered_map<std::string, std::string> _keyValMap;
	std::unordered_map<std::string, adbase::Timestamp> _ttlMap;
};
}
#endif
