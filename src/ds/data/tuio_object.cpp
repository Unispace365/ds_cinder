#include "stdafx.h"

#include "ds/data/tuio_object.h"

namespace ds {

/**
 * \class TuioObject
 */

TuioObject::TuioObject(int objectId, const ci::vec2& position, float angle, const ci::vec2& velocity,
					   float rotationVelocity)
  : mObjectId(objectId)
  , mPosition(position)
  , mAngle(angle)
  , mVelocity(velocity)
  , mRotationVelocity(rotationVelocity) {}

int TuioObject::getObjectId() const {
	return mObjectId;
}

const ci::vec2& TuioObject::getPosition() const {
	return mPosition;
}

float TuioObject::getAngle() const {
	return mAngle;
}

const ci::vec2& TuioObject::getVelocity() const {
	return mVelocity;
}

float TuioObject::getRotationVelocity() const {
	return mRotationVelocity;
}

} // namespace ds
