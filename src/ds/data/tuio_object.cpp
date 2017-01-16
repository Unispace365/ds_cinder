#include "stdafx.h"

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

TuioObject::TuioObject(const int objectId, const cinder::vec2& position, const float angle, const cinder::vec2& velocity, const float rotationVelocity)
		: mObjectId(objectId)
		, mPosition(position)
		, mAngle(angle)
		, mVelocity(velocity)
		, mRotationVelocity(rotationVelocity)
{
}

int TuioObject::getObjectId() const {
	return mObjectId;
}

const cinder::vec2& TuioObject::getPosition() const {
	return mPosition;
}

const float TuioObject::getAngle() const {
	return mAngle;
}

const cinder::vec2& TuioObject::getVelocity() const {
	return mVelocity;
}

const float TuioObject::getRotationVelocity() const {
	return mRotationVelocity;
}

} // namespace ds
