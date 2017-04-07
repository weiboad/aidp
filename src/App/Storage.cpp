#include "Storage.hpp"

namespace app {
// {{{ Storage::Storage()

Storage::Storage(AdbaseConfig* configure): _configure(configure) {
}

// }}}
// {{{ Storage::~Storage()

Storage::~Storage() {
}

// }}}
// {{{ void Storage::init()

void Storage::init() {
	adbase::metrics::Metrics::buildGauges("storage", "incrmap.size", 1000, [this](){
		std::lock_guard<std::mutex> lk(_mut);
		return _incrMap.size();
	});
	adbase::metrics::Metrics::buildGauges("storage", "keyvalmap.size", 1000, [this](){
		std::lock_guard<std::mutex> lk(_mut);
		return _keyValMap.size();
	});
	adbase::metrics::Metrics::buildGauges("storage", "ttlmap.size", 1000, [this](){
		std::lock_guard<std::mutex> lk(_mut);
		return _ttlMap.size();
	});
}

// }}}
// {{{ void Storage::clear()

void Storage::clear() {
	std::lock_guard<std::mutex> lk(_mut);
	adbase::Timestamp current = adbase::Timestamp::now();
	std::list<std::string> deleteKeys;
	for (auto &t : _ttlMap) {
		if (t.second < current) {
			continue;
		}
		deleteKeys.push_back(t.first);
	}
	for (auto &t : deleteKeys) {
		if (_keyValMap.find(t) != _keyValMap.end()) {
			_keyValMap.erase(t);	
		}	
		if (_ttlMap.find(t) != _ttlMap.end()) {
			_ttlMap.erase(t);	
		}	
	}
}

// }}}
// {{{ void Storage::bindClass()

void Storage::bindClass(adbase::lua::Engine* engine) {
	adbase::lua::BindingClass<Storage> storage("storage", "aidp", engine->getLuaState());
	adbase::lua::BindingNamespace storageCs = storage.getOwnerNamespace();
	typedef std::function<std::weak_ptr<Storage>()> GetStorage;
	GetStorage storageFn = std::bind(&Storage::getStorage, this);
	storageCs.addMethod("storage", storageFn);

	storage.addMethod<bool, Storage, std::string, std::string>("set", &Storage::set);
	storage.addMethod<bool, Storage, std::string, std::string, int>("set", &Storage::set);
	storage.addMethod("incr", &Storage::incr);
	storage.addMethod("decr", &Storage::decr);
	storage.addMethod("get", &Storage::get);
	storage.addMethod("exists", &Storage::exists);
	storage.addMethod("del", &Storage::deleteKey);
}

// }}}
// {{{ std::weak_ptr<Storage> Storage::getStorage() 

std::weak_ptr<Storage> Storage::getStorage() {
	return shared_from_this();
}

// }}}
// {{{ bool Storage::set()

bool Storage::set(const std::string key, const std::string value) {
	std::lock_guard<std::mutex> lk(_mut);
	_keyValMap[key] = value;
	return true;
}

// }}}
// {{{ bool Storage::set()

bool Storage::set(const std::string key, const std::string value, int ttl) {
	std::lock_guard<std::mutex> lk(_mut);
	_keyValMap[key] = value;
	_ttlMap[key] = adbase::addTime(adbase::Timestamp::now(), ttl);
	return true;
}

// }}}
// {{{ int64_t Storage::incr()

int64_t Storage::incr(const std::string key, int step) {
	std::lock_guard<std::mutex> lk(_mut);
	if (_incrMap.find(key) == _incrMap.end()) {
		_incrMap[key] = step;
	} else {
		_incrMap[key] += step;
	}
	return _incrMap[key];
}

// }}}
// {{{ int64_t Storage::decr()

int64_t Storage::decr(const std::string key, int step) {
	std::lock_guard<std::mutex> lk(_mut);
	if (_incrMap.find(key) == _incrMap.end()) {
		_incrMap[key] = -step;
	} else {
		_incrMap[key] -= step;
	}
	return _incrMap[key];
}

// }}}
// {{{ const std::string Storage::get()

const std::string Storage::get(const std::string key) {
	std::lock_guard<std::mutex> lk(_mut);
	if (_keyValMap.find(key) != _keyValMap.end()) {
		return _keyValMap[key];	
	}	
	if (_incrMap.find(key) != _incrMap.end()) {
		return std::to_string(_incrMap[key]);	
	}
	return "";
}

// }}}
// {{{ bool Storage::exists()

bool Storage::exists(const std::string key) {
	std::lock_guard<std::mutex> lk(_mut);
	if (_keyValMap.find(key) != _keyValMap.end()) {
		return true;	
	}	
	if (_incrMap.find(key) != _incrMap.end()) {
		return true;	
	}
	return false;
}

// }}}
// {{{ bool Storage::deleteKey()

bool Storage::deleteKey(const std::string key) {
	std::lock_guard<std::mutex> lk(_mut);
	if (_keyValMap.find(key) != _keyValMap.end()) {
		_keyValMap.erase(key);	
	}	
	if (_ttlMap.find(key) != _ttlMap.end()) {
		_ttlMap.erase(key);	
	}	
	if (_incrMap.find(key) != _incrMap.end()) {
		_incrMap.erase(key);
	}
	return true;
}

// }}}
}
