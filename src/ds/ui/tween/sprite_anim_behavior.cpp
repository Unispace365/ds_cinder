#include "sprite_anim_behavior.h"

#include "ds/ui/sprite/sprite.h"
#include "ds/util/string_util.h"
#include "cinder/Rand.h"

namespace {
static const std::string hash(const int sprite_id) {
	uint32_t a = static_cast<uint32_t>(ci::randInt() + sprite_id);
	a = (a ^ 61) ^ (a >> 16);
	a = a + (a << 3);
	a = a ^ (a >> 4);
	a = a * 0x27d4eb2d;
	a = a ^ (a >> 15);
	return ds::value_to_string(a);
}
}

namespace ds {
namespace ui {

SpriteAnimatableBehaviors::SpriteAnimatableBehaviors(Sprite& s)
	: mOwner(s)
	, mEventChannel("SpriteTweenEvent_" + hash(s.getId()))
{}

SpriteAnimatableBehaviors::~SpriteAnimatableBehaviors() {}

void SpriteAnimatableBehaviors::remove(const std::string& key) {
	if (contains(key)) mAnimatedProperties.erase(key);
};

bool SpriteAnimatableBehaviors::contains(const std::string& key) {
	return mAnimatedProperties.find(key) != mAnimatedProperties.end();
}

const std::string& SpriteAnimatableBehaviors::getChannelName() const {
	return mEventChannel;
}

} //!namespace ui
} //!namespace ds