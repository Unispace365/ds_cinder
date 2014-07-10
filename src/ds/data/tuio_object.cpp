#include "ds/data/tuio_object.h"

namespace ds {

/**
 * \class ds::TuioObject
 */
TuioObject::TuioObject()
		: mObjectId(-1)
		, mAngle(0.0f)
{
}

TuioObject::TuioObject(const int objectId, const cinder::Vec2f& position, const float angle)
		: mObjectId(objectId)
		, mPosition(position)
		, mAngle(angle)
{
}

int TuioObject::getObjectId() const {
	return mObjectId;
}

const cinder::Vec2f& TuioObject::getPosition() const {
	return mPosition;
}

const float TuioObject::getAngle() const {
	return mAngle;
}

} // namespace ds
