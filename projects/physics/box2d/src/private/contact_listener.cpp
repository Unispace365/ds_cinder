#include "private/contact_listener.h"

#include "Box2D/Dynamics/Contacts/b2Contact.h"

namespace ds {
namespace physics {

/**
 * \class ds::physics::ContactListener
 */
ContactListener::ContactListener()
{
}

void ContactListener::BeginContact(b2Contact* c)
{
	if (!c) return;
	collide(c->GetFixtureA());
	collide(c->GetFixtureB());
}

void ContactListener::setCollisionCallback(const ds::ui::Sprite& s, const std::function<void(void)>& fn)
{
	// Store a list of all sprites that need to be reported.
	// The alternative is to directly report to the sprite here,
	// but it's possible for the physics to collide the same
	// object many times in one update, so weed that out.
	try {
		if (!fn) {
			auto	found = mRegistered.find(&s);
			if (found != mRegistered.end()) mRegistered.erase(found);
		} else {
			mRegistered[&s] = fn;
		}
	} catch (std::exception const&) {
	}
}

void ContactListener::clear()
{
	mReport.clear();
}

void ContactListener::report()
{
	if (mReport.empty() || mRegistered.empty()) {
		mReport.clear();
		return;
	}

	for (auto it=mReport.begin(), end=mReport.end(); it!=end; ++it) {
		auto found = mRegistered.find(*it);
		if (found != mRegistered.end() && found->second) {
			found->second();
		}
	}
	mReport.clear();
}

void ContactListener::collide(const b2Fixture* fix)
{
	const b2Body*			b = (fix ? fix->GetBody() : nullptr);
	const ds::ui::Sprite*	sprite = reinterpret_cast<ds::ui::Sprite*>(b ? b->GetUserData() : nullptr);
	if (sprite) {
		mReport.insert(sprite);
	}
}

} // namespace physics
} // namespace ds
