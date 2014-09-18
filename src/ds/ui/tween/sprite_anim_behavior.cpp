#include "sprite_anim_behavior.h"

#include "ds/ui/sprite/sprite_engine.h"
#include "ds/ui/sprite/sprite.h"
#include "ds/util/string_util.h"
#include "ds/app/event_notifier.h"

#include "cinder/Rand.h"

namespace {
static const std::string hash() {
	uint32_t a = static_cast<uint32_t>(ci::randInt());
	a = (a ^ 61) ^ (a >> 16);
	a = a + (a << 3);
	a = a ^ (a >> 4);
	a = a * 0x27d4eb2d;
	a = a ^ (a >> 15);
	return ds::value_to_string(a);
}

static const auto EMPTY_LAMBDA = [](const std::string&){};

}

namespace ds {
namespace ui {

SpriteAnimatableBehaviors::SpriteAnimatableBehaviors(Sprite& s)
	: mOwner(s)
	, mEventChannelName("SpriteTweenEvent" + ds::value_to_string(s.getId()) + hash())
	, mEventNotifier(s.getEngine().getChannel(mEventChannelName))
	, mEventClient(mEventNotifier, [this](const ds::Event *e){ onEventNotify(e); })
	, mTweenStart(EMPTY_LAMBDA)
	, mTweenChange(EMPTY_LAMBDA)
	, mTweenEnd(EMPTY_LAMBDA)
	, mTweenAdded(EMPTY_LAMBDA)
	, mTweenRemoved(EMPTY_LAMBDA)
{}

SpriteAnimatableBehaviors::~SpriteAnimatableBehaviors() {}

void SpriteAnimatableBehaviors::remove(const std::string& key) {
	if (contains(key)) {
		mAnimatedProperties.erase(key);
		mEventNotifier.notify(TweenEventRemoved(key));
	}
};

bool SpriteAnimatableBehaviors::contains(const std::string& key) {
	return mAnimatedProperties.find(key) != mAnimatedProperties.end();
}

const std::string& SpriteAnimatableBehaviors::getChannelName() const {
	return mEventChannelName;
}

void SpriteAnimatableBehaviors::onEventNotify(const ds::Event* e) {
	if (!e) return;
	auto castedEvent = static_cast<const TweenEventStart*>(e);
	if (e->mWhat == TweenEventAdded::WHAT())
		mTweenAdded(castedEvent->key);
	if (e->mWhat == TweenEventRemoved::WHAT())
		mTweenRemoved(castedEvent->key);
	if (e->mWhat == TweenEventStart::WHAT())
		mTweenStart(castedEvent->key);
	if (e->mWhat == TweenEventEnd::WHAT())
		mTweenEnd(castedEvent->key);
	if (e->mWhat == TweenEventChange::WHAT())
		mTweenChange(castedEvent->key);
}

void SpriteAnimatableBehaviors::setAddFn(std::function<void(const std::string&)> Fn) { mTweenAdded.swap(Fn); }
void SpriteAnimatableBehaviors::setChangeFn(std::function<void(const std::string&)> Fn) { mTweenChange.swap(Fn); }
void SpriteAnimatableBehaviors::setRemoveFn(std::function<void(const std::string&)> Fn) { mTweenRemoved.swap(Fn); }
void SpriteAnimatableBehaviors::setStartFn(std::function<void(const std::string&)> Fn) { mTweenStart.swap(Fn); }
void SpriteAnimatableBehaviors::setEndFn(std::function<void(const std::string&)> Fn) { mTweenEnd.swap(Fn); }

} //!namespace ui
} //!namespace ds