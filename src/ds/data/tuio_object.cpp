#include "ds/data/tuio_object.h"

namespace ds {

/**
 * \class ds::TuioObject
 */
TuioObject::TuioObject()
		: mObjectId(-1) {
}

TuioObject::TuioObject(const int objectId, const cinder::Vec2f& position)
		: mObjectId(objectId)
		, mPosition(position) {
}

int TuioObject::getObjectId() const {
	return mObjectId;
}

const cinder::Vec2f& TuioObject::getPosition() const {
	return mPosition;
}

} // namespace ds
