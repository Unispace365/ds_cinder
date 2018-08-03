#pragma once
#ifndef DS_DATA_TUIOOBJECT_H_
#define DS_DATA_TUIOOBJECT_H_

#include <cinder/Vector.h>

namespace ds {

/**
 * \class TuioObject
 * \brief A wrapper around the TUIO object, so the
 * TUIO system doesn't have to be exposed to the app.
 */
class TuioObject {
public:
	TuioObject();
	TuioObject(const int objectId, const cinder::vec2& position, const float angle = 0.0f, const cinder::vec2& velocity = cinder::vec2(), const float rotationVelocity = 0.0f);

	int						getObjectId() const;
	const cinder::vec2&	getPosition() const;
	const float				getAngle() const;
	const cinder::vec2&	getVelocity() const;
	const float				getRotationVelocity() const;

private:
	int						mObjectId;
	cinder::vec2			mPosition;
	float					mAngle;
	cinder::vec2			mVelocity;
	float					mRotationVelocity;
};

} // namespace ds

#endif // DS_DATA_TUIOOBJECT_H_
