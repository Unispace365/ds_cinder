#pragma once
#ifndef DS_DATA_USERDATA_H_
#define DS_DATA_USERDATA_H_

#include "ds/data/key_value_store.h"

namespace ds {

/**
 * \class ds::UserData
 * \brief Arbitrary data supplied by a user, with a dynamically allocated
 * data store (i.e. very efficient for classes with a lot of subclasses,
 * only a few of which will have user data).
 */
class UserData {
public:
	UserData();

	// The API allows for indexed entries, but currently that's not supported.
	// It's not intended to ever be supported, it's just super annoying
	// to retrofit, just in case.
	// In all cases if an error value is not supplied then I throw on missing.
	float					getFloat(const std::string& key, const size_t index = 0) const;
	float					getFloat(const std::string& key, const size_t index, const float notFound) const;
	std::int32_t			getInt(const std::string& key, const size_t index = 0) const;
	std::int32_t			getInt(const std::string& key, const size_t index, const std::int32_t notFound) const;

	void					setFloat(const std::string& key, const float value, const size_t index = 0);
	void					setInt(const std::string& key, const std::int32_t value, const size_t index = 0);

private:
	std::unique_ptr<KeyValueStore>
							mStore;
};

} // namespace ds

#endif // DS_DATA_USERDATA_H_