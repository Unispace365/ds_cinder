#include "stdafx.h"

#include "ds/app/blob_registry.h"

#include "ds/debug/debug_defines.h"
#include <assert.h>
#include <iostream>

namespace ds {

BlobRegistry::BlobRegistry() {
	mReader.reserve(32);
	// Install an empty handler at index 0, because we don't want anyone
	// assigned byte '0' for transport.
	mReader.push_back(nullptr);
}

char BlobRegistry::add(const std::function<void(BlobReader&)>& reader) {
	assert(mReader.size() < 120);
	const char index = static_cast<char>(mReader.size());
	mReader.push_back(reader);
	return index;
}

} // namespace ds