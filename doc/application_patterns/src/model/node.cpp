#include "node.h"

#include <iostream>

namespace na {

namespace {
const std::wstring			EMPTY_WSZ;
}

/**
 * \class na::NodeRef::Data
 */
class NodeRef::Data {
public:
	Data()					{ }

	std::wstring			mName;
};

/**
 * \class na::NodeRef
 */
NodeRef::NodeRef() {
}

NodeRef::NodeRef(const std::wstring& name) {
	setName(name);
}

bool NodeRef::empty() const {
	return !mData;
}

void NodeRef::clear() {
	mData.reset();
}

const std::wstring& NodeRef::getName() const {
	if (!mData) return EMPTY_WSZ;
	return mData->mName;
}

NodeRef& NodeRef::setName(const std::string& n) {
	if (!mData) mData.reset(new Data());
	if (mData) mData->mName = n;
	return *this;
}

} // namespace na
