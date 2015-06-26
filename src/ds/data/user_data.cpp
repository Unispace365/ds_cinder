#include "ds/data/user_data.h"

#include <stdexcept>

namespace ds {

UserData::UserData() {
}

float UserData::getFloat(const std::string& key, const size_t index) const {
	if (!mStore) throw std::invalid_argument("Key " + key + " is invalid");
	return mStore->getFloat(key, index);
}

float UserData::getFloat(const std::string& key, const size_t index, const float notFound) const {
	try {
		return getFloat(key, index);
	} catch (std::exception const&) {
	}
	return notFound;
}

std::int32_t UserData::getInt(const std::string& key, const size_t index) const {
	if (!mStore) throw std::invalid_argument("Key " + key + " is invalid");
	return mStore->getInt(key, index);
}

std::int32_t UserData::getInt(const std::string& key, const size_t index, const std::int32_t notFound) const {
	try {
		return getInt(key, index);
	} catch (std::exception const&) {
	}
	return notFound;
}

void UserData::setFloat(const std::string& key, const float value, const size_t index) {
	try {
		if (!mStore) mStore.reset(new KeyValueStore());
		if (mStore) mStore->setFloat(key, value, index);
	} catch (std::exception const&) {
	}
}

void UserData::setInt(const std::string& key, const std::int32_t value, const size_t index) {
	try {
		if (!mStore) mStore.reset(new KeyValueStore());
		if (mStore) mStore->setInt(key, value, index);
	} catch (std::exception const&) {
	}
}

} // namespace ds
