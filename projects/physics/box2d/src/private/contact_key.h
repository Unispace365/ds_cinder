#pragma once
#ifndef DS_PHYSICS_PRIVATE_CONTACTKEY_H_
#define DS_PHYSICS_PRIVATE_CONTACTKEY_H_

#include <functional>

class b2Fixture;

namespace ds {
namespace ui {
class Sprite;
}

namespace physics {

/**
 * \class ds::physics::ContactKey
 */
class ContactKey
{
public:
	ContactKey();
	ContactKey(const ds::ui::Sprite*, const b2Fixture*, const float force);

	bool					operator==(const ContactKey&) const;
	bool					operator!=(const ContactKey&) const;

	const ds::ui::Sprite*	mSprite;
	const b2Fixture*		mFixture;
	// Note: force is ignored in terms equality and hashing.
	const float				mForce;
};

} // namespace physics
} // namespace ds

// Make the resource ID available for hashing functions
namespace std {
	template<>
	struct hash<ds::physics::ContactKey> : public unary_function<ds::physics::ContactKey, size_t> {
		size_t operator()(const ds::physics::ContactKey& key) const {
			return (size_t)( (size_t)key.mSprite + (size_t)key.mFixture);
		}
	};
}

#endif // DS_PHYSICS_PRIVATE_CONTACTKEY_H_
