#include "stdafx.h"

#include "ds/data/user_data.h"

#include <stdexcept>

namespace ds {

UserData::UserData() {}

float UserData::getFloat(const std::string& key, const size_t index) const {
	if (!mStore) {
		DS_LOG_WARNING("Key " + key + " is invalid");
		return 0.0f;
	}
	return mStore->getFloat(key, index);
}

float UserData::getFloat(const std::string& key, const size_t index, const float notFound) const {
	if (!mStore) {
		return notFound;
	}
	return mStore->getFloat(key, index);
}

std::int32_t UserData::getInt(const std::string& key, const size_t index) const {
	if (!mStore) {
		DS_LOG_WARNING("Key " + key + " is invalid");
		return 0;
	}
	return mStore->getInt(key, index);
}

std::int32_t UserData::getInt(const std::string& key, const size_t index, const std::int32_t notFound) const {
	if (!mStore) {
		return notFound;
	}
	return mStore->getInt(key, index);
}

std::string UserData::getString(const std::string& key, const size_t index /*= 0*/,
								const std::string& defaultStr /*= ""*/) const {
	if (!mStore) return defaultStr;
	return mStore->getString(key, index);
}

void UserData::setFloat(const std::string& key, const float value, const size_t index) {
	try {
		if (!mStore) mStore.reset(new KeyValueStore());
		if (mStore) mStore->setFloat(key, value, index);
	} catch (std::exception const&) {}
}

void UserData::setInt(const std::string& key, const std::int32_t value, const size_t index) {
	try {
		if (!mStore) mStore.reset(new KeyValueStore());
		if (mStore) mStore->setInt(key, value, index);
	} catch (std::exception const&) {}
}

void UserData::setString(const std::string& key, const std::string& value, const size_t index /*= 0*/) {
	try {
		if (!mStore) mStore.reset(new KeyValueStore());
		if (mStore) mStore->setString(key, value, index);
	} catch (std::exception const&) {}
}

} // namespace ds
