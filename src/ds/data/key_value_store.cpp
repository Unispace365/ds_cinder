#include "stdafx.h"

#include "ds/data/key_value_store.h"

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
		if (!values.get()) throw std::runtime_error("Out of memory (KeyValueStore)");
	}

	std::unordered_map<std::string, T>*		ptr = values.get();
	(*ptr)[key] = value;
}

template <typename T>
void			set_equal(	const std::unique_ptr<std::unordered_map<std::string, T>>& src,
							std::unique_ptr<std::unordered_map<std::string, T>>& dst) {
	if (!src.get()) {
		dst.release();
		return;
	}
	if (!dst.get()) {
		dst.reset(new std::unordered_map<std::string, T>());
		if (!dst.get()) throw std::runtime_error("Out of memory (KeyValueStore)");
	}
	const std::unordered_map<std::string, T>*	src_ptr = src.get();
	std::unordered_map<std::string, T>*			dst_ptr = dst.get();
	(*dst) = (*src);
}

}

/**
 * ds::KeyValueStore
 */
KeyValueStore::KeyValueStore() {
}

KeyValueStore::KeyValueStore(const KeyValueStore& o) {
	*this = o;
}

KeyValueStore& KeyValueStore::operator=(const KeyValueStore& o) {
	if (this == &o) return *this;

	set_equal<ci::ColorA>(o.mColorA, mColorA);
	set_equal<float>(o.mFloat, mFloat);
	set_equal<std::int32_t>(o.mInt, mInt);

	return *this;
}

ci::ColorA KeyValueStore::getColorA(const std::string& key, const size_t index) const {
	return get_value<ci::ColorA>(mColorA, key, index);
}

ci::ColorA KeyValueStore::getColorA(const std::string& key, const size_t index, const ci::ColorA& notFound) const {
	try {
		return getColorA(key, index);
	} catch (std::exception const&) {
	}
	return notFound;
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
	try {
		return get_value<std::int32_t>(mInt, key, index);
	} catch (std::exception const&) {
		return 0;
	}
}

std::int32_t KeyValueStore::getInt(const std::string& key, const size_t index, const std::int32_t notFound) const {
	int32_t out = getInt(key, index);
	if (out == 0) {
		out = notFound;
	}
	return out;
}

std::string KeyValueStore::getString(const std::string& key, const size_t index) const {
	try {
		return get_value<std::string>(mString, key, index);
	} catch (std::exception const&) {
	}
	return "";
}

std::string KeyValueStore::getString(const std::string& key, const size_t index, const std::string& notFound) const {
	std::string found = getString(key, index);
	if(!found.empty()) {
		return found;
	} else {
		return notFound;
	}
}

void KeyValueStore::setColorA(const std::string& key, const ci::ColorA& value, const size_t index) {
	try {
		set_value<ci::ColorA>(mColorA, key, value, index);
	} catch (std::exception const&) {
	}
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

void KeyValueStore::setString(const std::string& key, const std::string& value, const size_t index) {
	try {
		set_value<std::string>(mString, key, value, index);
	} catch (std::exception const&) {
	}
}

} // namespace ds
