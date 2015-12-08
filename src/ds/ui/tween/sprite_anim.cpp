#include "ds/ui/tween/sprite_anim.h"

#include "ds/ui/sprite/sprite.h"
#include "ds/ui/sprite/sprite_engine.h"
#include "ds/ui/tween/tweenline.h"

#include "ds/util/string_util.h"

namespace ds {
namespace ui {

/**
 * \class ds::ui::SpriteAnimatable
 */
SpriteAnimatable::SpriteAnimatable(Sprite& s, SpriteEngine& e)
	: mOwner(s)
	, mEngine(e)
	, mAnimateOnTargetsSet(false)
	, mAnimateOnScaleTarget(1.0f, 1.0f, 1.0f)
	, mAnimateOnPositionTarget(0.0f, 0.0f, 0.0f)
	, mAnimateOnOpacityTarget(1.0f)
{}

SpriteAnimatable::~SpriteAnimatable() {
}

const SpriteAnim<ci::Color>& SpriteAnimatable::ANIM_COLOR() {
	static ds::ui::SpriteAnim<ci::Color>  ANIM(
		[](ds::ui::Sprite& s)->ci::Anim<ci::Color>& { return s.mAnimColor; },
		[](ds::ui::Sprite& s)->ci::Color { return s.getColor(); },
		[](const ci::Color& v, ds::ui::Sprite& s) { s.setColor(v); });
	return ANIM;
}

const SpriteAnim<float>& SpriteAnimatable::ANIM_OPACITY() {
	static ds::ui::SpriteAnim<float>  ANIM(
		[](ds::ui::Sprite& s)->ci::Anim<float>& { return s.mAnimOpacity; },
		[](ds::ui::Sprite& s)->float { return s.getOpacity(); },
		[](const float& v, ds::ui::Sprite& s) { s.setOpacity(v); });
	return ANIM;
}

const SpriteAnim<ci::Vec3f>& SpriteAnimatable::ANIM_POSITION() {
	static ds::ui::SpriteAnim<ci::Vec3f>  ANIM(
		[](ds::ui::Sprite& s)->ci::Anim<ci::Vec3f>& { return s.mAnimPosition; },
		[](ds::ui::Sprite& s)->ci::Vec3f { return s.getPosition(); },
		[](const ci::Vec3f& v, ds::ui::Sprite& s) { s.setPosition(v); });
	return ANIM;
}

const SpriteAnim<ci::Vec3f>& SpriteAnimatable::ANIM_SCALE() {
	static ds::ui::SpriteAnim<ci::Vec3f>  ANIM(
		[](ds::ui::Sprite& s)->ci::Anim<ci::Vec3f>& { return s.mAnimScale; },
		[](ds::ui::Sprite& s)->ci::Vec3f { return s.getScale(); },
		[](const ci::Vec3f& v, ds::ui::Sprite& s) { s.setScale(v); });
	return ANIM;
}

const SpriteAnim<ci::Vec3f>& SpriteAnimatable::ANIM_SIZE() {
	static ds::ui::SpriteAnim<ci::Vec3f>  ANIM(
		[](ds::ui::Sprite& s)->ci::Anim<ci::Vec3f>& { return s.mAnimSize; },
		[](ds::ui::Sprite& s)->ci::Vec3f { return ci::Vec3f(s.getWidth(), s.getHeight(), s.getDepth()); },
		[](const ci::Vec3f& v, ds::ui::Sprite& s) { s.setSizeAll(v.x, v.y, v.z); });
	return ANIM;
}

const SpriteAnim<ci::Vec3f>& SpriteAnimatable::ANIM_ROTATION() {
	static ds::ui::SpriteAnim<ci::Vec3f>  ANIM(
		[](ds::ui::Sprite& s)->ci::Anim<ci::Vec3f>& { return s.mAnimRotation; },
		[](ds::ui::Sprite& s)->ci::Vec3f { return s.getRotation(); },
		[](const ci::Vec3f& v, ds::ui::Sprite& s) { s.setRotation(v); });
	return ANIM;
}

void SpriteAnimatable::tweenColor(const ci::Color& c, const float duration, const float delay,
								  const ci::EaseFn& ease, const std::function<void(void)>& finishFn, const std::function<void(void)>& updateFn) {
	mAnimColor.stop();
	mEngine.getTweenline().apply(mOwner, ANIM_COLOR(), c, duration, ease, finishFn, delay, updateFn);
}

void SpriteAnimatable::tweenOpacity(const float opacity, const float duration, const float delay,
									const ci::EaseFn& ease, const std::function<void(void)>& finishFn, const std::function<void(void)>& updateFn) {
	mAnimOpacity.stop();
	mEngine.getTweenline().apply(mOwner, ANIM_OPACITY(), opacity, duration, ease, finishFn, delay, updateFn);
}

void SpriteAnimatable::tweenPosition(const ci::Vec3f& pos, const float duration, const float delay,
									 const ci::EaseFn& ease, const std::function<void(void)>& finishFn, const std::function<void(void)>& updateFn) {
	mAnimPosition.stop();
	mEngine.getTweenline().apply(mOwner, ANIM_POSITION(), pos, duration, ease, finishFn, delay, updateFn);
}

void SpriteAnimatable::tweenRotation(const ci::Vec3f& rot, const float duration, const float delay,
									 const ci::EaseFn& ease, const std::function<void(void)>& finishFn, const std::function<void(void)>& updateFn) {
	mAnimRotation.stop();
	mEngine.getTweenline().apply(mOwner, ANIM_ROTATION(), rot, duration, ease, finishFn, delay, updateFn);
}

void SpriteAnimatable::tweenScale(const ci::Vec3f& scale, const float duration, const float delay,
								  const ci::EaseFn& ease, const std::function<void(void)>& finishFn, const std::function<void(void)>& updateFn) {
	mAnimScale.stop();
	mEngine.getTweenline().apply(mOwner, ANIM_SCALE(), scale, duration, ease, finishFn, delay, updateFn);
}

void SpriteAnimatable::tweenSize(const ci::Vec3f& size, const float duration, const float delay,
								 const ci::EaseFn& ease, const std::function<void(void)>& finishFn, const std::function<void(void)>& updateFn) {
	mAnimSize.stop();
	mEngine.getTweenline().apply(mOwner, ANIM_SIZE(), size, duration, ease, finishFn, delay, updateFn);
}

void SpriteAnimatable::animStop() {
	mAnimColor.stop();
	mAnimOpacity.stop();
	mAnimPosition.stop();
	mAnimScale.stop();
	mAnimSize.stop();
}

void SpriteAnimatable::tweenAnimateOn(const bool recursive, const float delay, const float deltaDelay){
	float thisDelay = delay;
	runAnimationScript(mAnimateOnScript, thisDelay);
	if(recursive){
		auto chillins = mOwner.getChildren();
		for(auto it = chillins.begin(), end = chillins.end(); it != end; ++it) {
			Sprite*		s(*it);
			if(s) {
				thisDelay += deltaDelay;
				s->tweenAnimateOn(true, thisDelay, deltaDelay);
			}
		}
	}
}

void SpriteAnimatable::setAnimateOnScript(const std::string& animateOnScript){
	mAnimateOnScript = animateOnScript;
}

void SpriteAnimatable::setAnimateOnTargets(){
	mAnimateOnTargetsSet = true;
	mAnimateOnScaleTarget = mOwner.getScale();
	mAnimateOnPositionTarget = mOwner.getPosition();
	mAnimateOnOpacityTarget = mOwner.getOpacity();
}

void SpriteAnimatable::clearAnimateOnTargets(){
	mAnimateOnTargetsSet = false;
}

void SpriteAnimatable::runAnimationScript(const std::string& animScript, const float addedDelay){
	if(animScript.empty()) return;

	// find all the commands in the string
	std::vector<std::string> commands = ds::split(animScript, "; ", true);

	if(commands.empty()) return;

	// set default parameters, if they're not supplied by the string
	ci::EaseFn easing = ci::EaseInOutCubic();
	float dur = 0.35f;
	float delayey = addedDelay;

	// This maps tracks all the types (scale, position, etc) and their destinations (as 3d vectors)
	std::map<std::string, ci::Vec3f> animationCommands;
	for(auto it = commands.begin(); it < commands.end(); ++it){

		// Split commands between the type and the destination
		std::vector<std::string> commandProperties = ds::split((*it), ":", true);
		if(commandProperties.empty() || commandProperties.size() < 1) continue;

		// Parse out the special commands
		std::string keyey = commandProperties.front();
		if(keyey == "ease"){
			std::string easeString = commandProperties[1];
			easing = getEasingByString(easeString);
			continue;
		} else if(keyey == "duration"){
			ds::string_to_value<float>(commandProperties[1], dur);
			continue;
		} else if(keyey == "delay"){
			float rootDelay = 0.0f;
			ds::string_to_value<float>(commandProperties[1], rootDelay);
			delayey += rootDelay;
			continue;
		}

		// parse the destination vectors to floats
		ci::Vec3f destination = ci::Vec3f::zero();

		if(commandProperties.size() > 1){
			std::vector<std::string> destinationTokens = ds::split(commandProperties[1], ", ", true);

			ds::string_to_value<float>(destinationTokens[0], destination.x);
			if(destinationTokens.size() > 1){
				ds::string_to_value<float>(destinationTokens[1], destination.y);
			}
			if(destinationTokens.size() > 2){
				ds::string_to_value<float>(destinationTokens[2], destination.z);
			}
		}

		animationCommands[keyey] = destination;
	}

	// now that we have all the commands, apply them
	for(auto it = animationCommands.begin(); it != animationCommands.end(); ++it){
		std::string animType = it->first;
		ci::Vec3f dest = it->second;
		if(animType == "scale"){
			tweenScale(dest, dur, delayey, easing);
		} else if(animType == "opacity"){
			tweenOpacity(dest.x, dur, delayey, easing);
		} else if(animType == "position"){
			tweenPosition(dest, dur, delayey, easing);
		} else if(animType == "rotation"){
			tweenRotation(dest, dur, delayey, easing);
		} else if(animType == "size"){
			tweenSize(dest, dur, delayey, easing);
		} else if(animType == "color"){
			tweenColor(ci::Color(dest.x, dest.y, dest.z), dur, delayey, easing);

		} else if(animType == "slide"){
			if(!mAnimateOnTargetsSet) setAnimateOnTargets();
			mOwner.setPosition(mAnimateOnPositionTarget + dest);
			tweenPosition(mAnimateOnPositionTarget, dur, delayey, easing);

		} else if(animType == "fade"){
			if(!mAnimateOnTargetsSet) setAnimateOnTargets();
			if(dest.x == 0.0f){
				mOwner.setOpacity(0.0f);
			} else {
				mOwner.setOpacity(mAnimateOnOpacityTarget + dest.x);
			}
			tweenOpacity(mAnimateOnOpacityTarget, dur, delayey, easing);

		} else if(animType == "grow"){
			if(!mAnimateOnTargetsSet) setAnimateOnTargets();
			if(dest.x == 0.0f && dest.y == 0.0f){
				mOwner.setScale(0.0f);
			} else {
				mOwner.setScale(mAnimateOnScaleTarget + dest);
			}
			tweenScale(mAnimateOnScaleTarget, dur, delayey, easing);
		}
	}
}

ci::EaseFn SpriteAnimatable::getEasingByString(const std::string& inString){
	static std::map<std::string, ci::EaseFn> easings;
	if(easings.empty()){
		easings["none"] = ci::easeNone;
		easings["inQuad"] = ci::easeInQuad;
		easings["outQuad"] = ci::easeOutQuad;
		easings["inOutQuad"] = ci::easeInOutQuad;
		easings["inCubic"] = ci::easeInCubic;
		easings["outCubic"] = ci::easeOutCubic;
		easings["inOutCubic"] = ci::easeInOutCubic;
		easings["inQuart"] = ci::easeInQuart;
		easings["outQuart"] = ci::easeOutQuart;
		easings["inOutQuart"] = ci::easeInOutQuart;
		easings["inQuint"] = ci::easeInQuint;
		easings["outQuint"] = ci::easeOutQuint;
		easings["inOutQuint"] = ci::easeInOutQuint;
		easings["inSine"] = ci::easeInSine;
		easings["outSine"] = ci::easeOutSine;
		easings["inOutSine"] = ci::easeInOutSine;
		easings["inExpo"] = ci::easeInExpo;
		easings["outExpo"] = ci::easeOutExpo;
		easings["inOutExpo"] = ci::easeInOutExpo;
		easings["inCirc"] = ci::easeInCirc;
		easings["outCirc"] = ci::easeOutCirc;
		easings["inOutCirc"] = ci::easeInOutCirc;

		// These easings require instantiation with a default parameter
		easings["inBounce"] = ci::EaseInBounce();
		easings["outBounce"] = ci::EaseOutBounce();
		easings["inOutBounce"] = ci::EaseInOutBounce();
		easings["inBack"] = ci::EaseInBack();
		easings["outBack"] = ci::EaseOutBack();
		easings["inOutBack"] = ci::EaseInOutBack();
		easings["inAtan"] = ci::EaseInAtan();
		easings["outAtan"] = ci::EaseOutAtan();
		easings["inOutAtan"] = ci::EaseInOutAtan();

		// note: elastic easings require a parameter to be instantiated, so can't be used by this lookup method :(
		//easings["inElastic"] = ci::EaseInElastic();
		//easings["outElastic"] = ci::EaseOutElastic();
		//easings["inOutElastic"] = ci::EaseInOutElastic();
	}

	auto fit = easings.find(inString);
	if(fit != easings.end()){
		return fit->second;
	}

	return ci::easeNone;
}

} // namespace ui
} // namespace ds
