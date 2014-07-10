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
	TuioObject(const int objectId, const cinder::Vec2f& position, const float angle = 0.0f);

	int						getObjectId() const;
	const cinder::Vec2f&	getPosition() const;
	const float				getAngle() const;

private:
	int						mObjectId;
	cinder::Vec2f			mPosition;
	float					mAngle;
};

} // namespace ds

#endif // DS_DATA_TUIOOBJECT_H_
