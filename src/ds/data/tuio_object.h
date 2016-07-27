#pragma once
#ifndef DS_DATA_TUIOOBJECT_H_
#define DS_DATA_TUIOOBJECT_H_

#include <cinder/Vector.h>

namespace ds {

/**
 * \class ds::TuioObject
 * \brief A wrapper around the TUIO object, so the
 * TUIO system doesn't have to be exposed to the app.
 */
class TuioObject {
public:
	TuioObject();
	TuioObject(const int objectId, const cinder::Vec2f& position, const float angle = 0.0f, const cinder::Vec2f& velocity = cinder::Vec2f(), const float rotationVelocity = 0.0f);

	int						getObjectId() const;
	const cinder::Vec2f&	getPosition() const;
	const float				getAngle() const;
	const cinder::Vec2f&	getVelocity() const;
	const float				getRotationVelocity() const;

private:
	int						mObjectId;
	cinder::Vec2f			mPosition;
	float					mAngle;
	cinder::Vec2f			mVelocity;
	float					mRotationVelocity;
};

} // namespace ds

#endif // DS_DATA_TUIOOBJECT_H_
