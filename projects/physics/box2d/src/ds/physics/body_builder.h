#pragma once
#ifndef DS_PHYSICS_BODYBUILDER_H_
#define DS_PHYSICS_BODYBUILDER_H_

#include <vector>
#include <cinder/Vector.h>

namespace ds {
namespace physics {
class SpriteBody;

/**
 * \class ds::physics::BodyBuilder
 * \brief Used to supply parameters when creating a body.
 */
class BodyBuilder {
public:
	virtual ~BodyBuilder();

	virtual void		createFixture(SpriteBody&) const = 0;

	float				mDensity,
						mFriction,
						mLinearDampening,
						mAngularDampening;
	bool				mFixedRotation;
	bool				mIsStaticBody;
	int					mCategoryBits;
	int					mMaskBits;

protected:
	BodyBuilder(const SpriteBody&);
};

/**
 * \class ds::physics::BodyBuilderBox
 * \brief Create a box shape.
 */
class BodyBuilderBox : public BodyBuilder {
public:
	BodyBuilderBox(const SpriteBody&);

	virtual void		createFixture(SpriteBody&) const;

	float				mWidth,
						mHeight;
};

/**
 * \class ds::physics::BodyBuilderCircle
 * \brief Create a circle shape.
 */
class BodyBuilderCircle : public BodyBuilder {
public:
	BodyBuilderCircle(const SpriteBody&);

	virtual void		createFixture(SpriteBody&) const;

	float				mRadius;
};


/**
 * \class ds::physics::BodyBuilderPolygon
 * \brief Create an arbitrary polygon. See the box2d manual for specifics on polygon shapes
 */
class BodyBuilderPolygon : public BodyBuilder {
public:
	BodyBuilderPolygon(const SpriteBody&);

	virtual void		createFixture(SpriteBody&) const;

	 std::vector<ci::Vec2f> mPoints;
};

} // namespace physics
} // namespace ds

#endif // DS_PHYSICS_BODYBUILDER_H_
