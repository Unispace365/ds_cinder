#pragma once
#ifndef DS_CONTAINER_KEYVALUESTORE_H_
#define DS_CONTAINER_KEYVALUESTORE_H_

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ds {

/**
 * \class ds::KeyValueStore
 * \brief A generic data store. Intended to be relatively efficient for
 * sparse users, only allocating storage as needed.
 */
class KeyValueStore {
public:
	KeyValueStore();

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
	std::unique_ptr<std::unordered_map<std::string, float>>
							mFloat;
	std::unique_ptr<std::unordered_map<std::string, std::int32_t>>
							mInt;
};

} // namespace ds

#endif // DS_CONTAINER_KEYVALUESTORE_H_