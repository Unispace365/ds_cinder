#include "sprite_anim_behavior.h"

#include "ds/ui/sprite/sprite.h"
#include "ds/util/string_util.h"

namespace ds {
namespace ui {

SpriteAnimatableBehaviors::SpriteAnimatableBehaviors(Sprite& s)
	: mOwner(s)
	, mEventChannel("SpriteTweenEvent_" + ds::value_to_string(s.getId()))
{}

SpriteAnimatableBehaviors::~SpriteAnimatableBehaviors() {}

void SpriteAnimatableBehaviors::remove(const std::string& key) {
	if (contains(key)) mAnimatedProperties.erase(key);
};

bool SpriteAnimatableBehaviors::contains(const std::string& key) {
	return mAnimatedProperties.find(key) != mAnimatedProperties.end();
}

} //!namespace ui
} //!namespace ds