#pragma once
#ifndef DS_PHYSICS_BODYBUILDER_H_
#define DS_PHYSICS_BODYBUILDER_H_

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

} // namespace physics
} // namespace ds

#endif // DS_PHYSICS_BODYBUILDER_H_
