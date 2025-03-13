#pragma once
#ifndef DS_DATA_TUIOOBJECT_H_
#define DS_DATA_TUIOOBJECT_H_

namespace ds {

/**
 * \class TuioObject
 * \brief A wrapper around the TUIO object, so the
 * TUIO system doesn't have to be exposed to the app.
 */
class TuioObject {
  public:
	TuioObject() = default;
	TuioObject(int objectId, const cinder::vec2& position, float angle = 0.0f,
			   const cinder::vec2& velocity = cinder::vec2(), float rotationVelocity = 0.0f);

	int					getObjectId() const;
	const cinder::vec2& getPosition() const;
	float				getAngle() const;
	const cinder::vec2& getVelocity() const;
	float				getRotationVelocity() const;

  private:
	int			 mObjectId{-1};
	cinder::vec2 mPosition;
	float		 mAngle{0};
	cinder::vec2 mVelocity;
	float		 mRotationVelocity{0};
};

} // namespace ds

#endif // DS_DATA_TUIOOBJECT_H_
