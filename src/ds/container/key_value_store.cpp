#include "ds/container/key_value_store.h"

#include <stdexcept>

namespace ds {

namespace {

template <typename T>
T				get_value(const std::unique_ptr<std::unordered_map<std::string, T>>& values, const std::string& key, const size_t index) {
	if (!values.get()) throw std::invalid_argument("Key " + key + " is invalid");
	if (values->empty()) throw std::invalid_argument("Key " + key + " is invalid");
	const auto f = values->find(key);
	if (f == values->end()) throw std::invalid_argument("Key " + key + " is invalid");
	return f->second;
}

template <typename T>
void			set_value(std::unique_ptr<std::unordered_map<std::string, T>>& values, const std::string& key, const T& value, const size_t index) {
	if (!values.get()) {
		values.reset(new std::unordered_map<std::string, T>());
		if (!values.get()) throw std::runtime_error("Out of memory");
	}

	std::unordered_map<std::string, T>*		ptr = values.get();
	(*ptr)[key] = value;
}

}

/**
 * ds::KeyValueStore
 */
KeyValueStore::KeyValueStore() {
}

float KeyValueStore::getFloat(const std::string& key, const size_t index) const {
	return get_value<float>(mFloat, key, index);
}

float KeyValueStore::getFloat(const std::string& key, const size_t index, const float notFound) const {
	try {
		return getFloat(key, index);
	} catch (std::exception const&) {
	}
	return notFound;
}

std::int32_t KeyValueStore::getInt(const std::string& key, const size_t index) const {
	return get_value<std::int32_t>(mInt, key, index);
}

std::int32_t KeyValueStore::getInt(const std::string& key, const size_t index, const std::int32_t notFound) const {
	try {
		return getInt(key, index);
	} catch (std::exception const&) {
	}
	return notFound;
}

void KeyValueStore::setFloat(const std::string& key, const float value, const size_t index) {
	try {
		set_value<float>(mFloat, key, value, index);
	} catch (std::exception const&) {
	}
}

void KeyValueStore::setInt(const std::string& key, const std::int32_t value, const size_t index) {
	try {
		set_value<std::int32_t>(mInt, key, value, index);
	} catch (std::exception const&) {
	}
}

} // namespace ds
