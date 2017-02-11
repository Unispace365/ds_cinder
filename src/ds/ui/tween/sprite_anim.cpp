#include "stdafx.h"

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
	, mAnimateOnScript("")
	, mNormalizedTweenValue(0.0f)
	, mInternalColorCinderTweenRef(nullptr)
	, mInternalScaleCinderTweenRef(nullptr)
	, mInternalRotationCinderTweenRef(nullptr)
	, mInternalPositionCinderTweenRef(nullptr)
	, mInternalSizeCinderTweenRef(nullptr)
	, mInternalOpacityCinderTweenRef(nullptr)
	, mInternalNormalizedCinderTweenRef(nullptr)
{}

SpriteAnimatable::~SpriteAnimatable() {
	mInternalColorCinderTweenRef = nullptr;
	mInternalScaleCinderTweenRef = nullptr;
	mInternalRotationCinderTweenRef = nullptr;
	mInternalPositionCinderTweenRef = nullptr;
	mInternalSizeCinderTweenRef = nullptr;
	mInternalOpacityCinderTweenRef = nullptr;
	mInternalNormalizedCinderTweenRef = nullptr;
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

const SpriteAnim<ci::vec3>& SpriteAnimatable::ANIM_POSITION() {
	static ds::ui::SpriteAnim<ci::vec3>  ANIM(
		[](ds::ui::Sprite& s)->ci::Anim<ci::vec3>& { return s.mAnimPosition; },
		[](ds::ui::Sprite& s)->ci::vec3 { return s.getPosition(); },
		[](const ci::vec3& v, ds::ui::Sprite& s) { s.setPosition(v); });
	return ANIM;
}

const SpriteAnim<ci::vec3>& SpriteAnimatable::ANIM_SCALE() {
	static ds::ui::SpriteAnim<ci::vec3>  ANIM(
		[](ds::ui::Sprite& s)->ci::Anim<ci::vec3>& { return s.mAnimScale; },
		[](ds::ui::Sprite& s)->ci::vec3 { return s.getScale(); },
		[](const ci::vec3& v, ds::ui::Sprite& s) { s.setScale(v); });
	return ANIM;
}

const SpriteAnim<ci::vec3>& SpriteAnimatable::ANIM_SIZE() {
	static ds::ui::SpriteAnim<ci::vec3>  ANIM(
		[](ds::ui::Sprite& s)->ci::Anim<ci::vec3>& { return s.mAnimSize; },
		[](ds::ui::Sprite& s)->ci::vec3 { return ci::vec3(s.getWidth(), s.getHeight(), s.getDepth()); },
		[](const ci::vec3& v, ds::ui::Sprite& s) { s.setSizeAll(v.x, v.y, v.z); });
	return ANIM;
}

const SpriteAnim<ci::vec3>& SpriteAnimatable::ANIM_ROTATION() {
	static ds::ui::SpriteAnim<ci::vec3>  ANIM(
		[](ds::ui::Sprite& s)->ci::Anim<ci::vec3>& { return s.mAnimRotation; },
		[](ds::ui::Sprite& s)->ci::vec3 { return s.getRotation(); },
		[](const ci::vec3& v, ds::ui::Sprite& s) { s.setRotation(v); });
	return ANIM;
}

const SpriteAnim<float>& SpriteAnimatable::ANIM_NORMALIZED() {
	static ds::ui::SpriteAnim<float>  ANIM(
		[](ds::ui::Sprite& s)->ci::Anim<float>& { s.mNormalizedTweenValue = 0.0f; return s.mAnimNormalized; },
		[](ds::ui::Sprite& s)->float { return s.mNormalizedTweenValue; },
		[](const float& v, ds::ui::Sprite& s) { s.mNormalizedTweenValue = v; });
	return ANIM;
}

void SpriteAnimatable::tweenColor(const ci::Color& c, const float duration, const float delay,
								  const ci::EaseFn& ease, const std::function<void(void)>& finishFn, const std::function<void(void)>& updateFn) {
	animColorStop();
	auto options = mEngine.getTweenline().apply(mOwner, ANIM_COLOR(), c, duration, ease, finishFn, delay, updateFn);
	mInternalColorCinderTweenRef = options.operator ci::TweenRef<ci::Color>();
}

void SpriteAnimatable::tweenOpacity(const float opacity, const float duration, const float delay,
									const ci::EaseFn& ease, const std::function<void(void)>& finishFn, const std::function<void(void)>& updateFn) {
	animOpacityStop();
	auto options = mEngine.getTweenline().apply(mOwner, ANIM_OPACITY(), opacity, duration, ease, finishFn, delay, updateFn);
	mInternalOpacityCinderTweenRef = options.operator ci::TweenRef<float>();
}

void SpriteAnimatable::tweenPosition(const ci::vec3& pos, const float duration, const float delay,
									 const ci::EaseFn& ease, const std::function<void(void)>& finishFn, const std::function<void(void)>& updateFn) {
	animPositionStop();
	auto options = mEngine.getTweenline().apply(mOwner, ANIM_POSITION(), pos, duration, ease, finishFn, delay, updateFn);
	mInternalPositionCinderTweenRef = options.operator ci::TweenRef<ci::vec3>();
}

void SpriteAnimatable::tweenRotation(const ci::vec3& rot, const float duration, const float delay,
									 const ci::EaseFn& ease, const std::function<void(void)>& finishFn, const std::function<void(void)>& updateFn) {
	animRotationStop();
	auto options = mEngine.getTweenline().apply(mOwner, ANIM_ROTATION(), rot, duration, ease, finishFn, delay, updateFn);
	mInternalRotationCinderTweenRef = options.operator ci::TweenRef<ci::vec3>();
}

void SpriteAnimatable::tweenScale(const ci::vec3& scale, const float duration, const float delay,
								  const ci::EaseFn& ease, const std::function<void(void)>& finishFn, const std::function<void(void)>& updateFn) {
	animScaleStop();
	auto options = mEngine.getTweenline().apply(mOwner, ANIM_SCALE(), scale, duration, ease, finishFn, delay, updateFn);
	mInternalScaleCinderTweenRef = options.operator ci::TweenRef<ci::vec3>();
}

void SpriteAnimatable::tweenSize(const ci::vec3& size, const float duration, const float delay,
								 const ci::EaseFn& ease, const std::function<void(void)>& finishFn, const std::function<void(void)>& updateFn) {
	animSizeStop();
	auto options = mEngine.getTweenline().apply(mOwner, ANIM_SIZE(), size, duration, ease, finishFn, delay, updateFn);
	mInternalSizeCinderTweenRef = options.operator ci::TweenRef<ci::vec3>();
}

void SpriteAnimatable::tweenNormalized(const float duration, const float delay,
									const ci::EaseFn& ease, const std::function<void(void)>& finishFn, const std::function<void(void)>& updateFn) {
	animNormalizedStop();
	auto options = mEngine.getTweenline().apply(mOwner, ANIM_NORMALIZED(), 1.0f, duration, ease, finishFn, delay, updateFn);
	mInternalNormalizedCinderTweenRef = options.operator ci::TweenRef<float>();
}


void SpriteAnimatable::completeTweenColor(const bool callFinishFunction){
	if(mInternalColorCinderTweenRef){
		if(getColorTweenIsRunning()){
			mAnimColor.stop();
			mOwner.setColor(mInternalColorCinderTweenRef->getEndValue());
			if(callFinishFunction){
				auto finishFunc = mInternalColorCinderTweenRef->getFinishFn();
				if(finishFunc) finishFunc();
			}
		}
	}
}

void SpriteAnimatable::completeTweenOpacity(const bool callFinishFunction){
	if(mInternalOpacityCinderTweenRef){
		if(getOpacityTweenIsRunning()){
			mAnimOpacity.stop();
			mOwner.setOpacity(mInternalOpacityCinderTweenRef->getEndValue());
			if(callFinishFunction){
				auto finishFunc = mInternalOpacityCinderTweenRef->getFinishFn();
				if(finishFunc) finishFunc();
			}
		}
	}
}

void SpriteAnimatable::completeTweenPosition(const bool callFinishFunction){
	if(mInternalPositionCinderTweenRef){
		if(getPositionTweenIsRunning()){
			mAnimPosition.stop();
			mOwner.setPosition(mInternalPositionCinderTweenRef->getEndValue());
			if(callFinishFunction){
				auto finishFunc = mInternalPositionCinderTweenRef->getFinishFn();
				if(finishFunc) finishFunc();
			}
		}
	}
}

void SpriteAnimatable::completeTweenRotation(const bool callFinishFunction){
	if(mInternalRotationCinderTweenRef){
		if(getRotationTweenIsRunning()){
			mAnimRotation.stop();
			mOwner.setRotation(mInternalRotationCinderTweenRef->getEndValue());
			if(callFinishFunction){
				auto finishFunc = mInternalRotationCinderTweenRef->getFinishFn();
				if(finishFunc) finishFunc();
			}
		}
	}
}

void SpriteAnimatable::completeTweenScale(const bool callFinishFunction){
	if(mInternalScaleCinderTweenRef){
		if(getScaleTweenIsRunning()){
			mAnimScale.stop();
			mOwner.setScale(mInternalScaleCinderTweenRef->getEndValue());
			if(callFinishFunction){
				auto finishFunc = mInternalScaleCinderTweenRef->getFinishFn();
				if(finishFunc) finishFunc();
			}
		}
	}
}

void SpriteAnimatable::completeTweenSize(const bool callFinishFunction){
	if(mInternalSizeCinderTweenRef){
		if(getSizeTweenIsRunning()){
			mAnimSize.stop();
			mOwner.setSizeAll(mInternalSizeCinderTweenRef->getEndValue());
			if(callFinishFunction){
				auto finishFunc = mInternalSizeCinderTweenRef->getFinishFn();
				if(finishFunc) finishFunc();
			}
		}
	}
}

void SpriteAnimatable::completeTweenNormalized(const bool callFinishFunction){
	if(mInternalNormalizedCinderTweenRef){
		if(getSizeTweenIsRunning()){
			mAnimSize.stop();
			mNormalizedTweenValue = (mInternalNormalizedCinderTweenRef->getEndValue());
			if(callFinishFunction){
				auto finishFunc = mInternalNormalizedCinderTweenRef->getFinishFn();
				if(finishFunc) finishFunc();
			}
		}
	}
}
const bool SpriteAnimatable::animationRunning(){
	return getOpacityTweenIsRunning() 
		|| getPositionTweenIsRunning() 
		|| getScaleTweenIsRunning() 
		|| getSizeTweenIsRunning() 
		|| getRotationTweenIsRunning() 
		|| getColorTweenIsRunning() 
		|| getNormalizeTweenIsRunning();
}

const bool SpriteAnimatable::getPositionTweenIsRunning(){
	return (mInternalPositionCinderTweenRef && !mAnimPosition.isComplete());
}
const bool SpriteAnimatable::getRotationTweenIsRunning(){
	return (mInternalRotationCinderTweenRef && !mAnimRotation.isComplete());
}
const bool SpriteAnimatable::getSizeTweenIsRunning(){
	return (mInternalSizeCinderTweenRef && !mAnimSize.isComplete());
}
const bool SpriteAnimatable::getScaleTweenIsRunning(){
	return (mInternalScaleCinderTweenRef && !mAnimScale.isComplete());
}
const bool SpriteAnimatable::getOpacityTweenIsRunning(){
	return (mInternalOpacityCinderTweenRef && !mAnimOpacity.isComplete());
}
const bool SpriteAnimatable::getColorTweenIsRunning(){
	return (mInternalColorCinderTweenRef && !mAnimColor.isComplete());
}
const bool SpriteAnimatable::getNormalizeTweenIsRunning(){
	return (mInternalNormalizedCinderTweenRef && !mAnimNormalized.isComplete());
}

void SpriteAnimatable::animStop() {
	animPositionStop();
	animRotationStop();
	animScaleStop();
	animSizeStop();
	animOpacityStop();
	animColorStop();
	animNormalizedStop();
}

void SpriteAnimatable::animPositionStop(){
	mAnimPosition.stop();
	mInternalPositionCinderTweenRef = nullptr;
}

void SpriteAnimatable::animRotationStop(){
	mAnimRotation.stop();
	mInternalRotationCinderTweenRef = nullptr;
}

void SpriteAnimatable::animScaleStop(){
	mAnimScale.stop();
	mInternalScaleCinderTweenRef = nullptr;
}

void SpriteAnimatable::animSizeStop(){
	mAnimSize.stop();
	mInternalSizeCinderTweenRef = nullptr;
}

void SpriteAnimatable::animOpacityStop(){
	mAnimOpacity.stop();
	mInternalOpacityCinderTweenRef = nullptr;
}

void SpriteAnimatable::animColorStop(){
	mAnimColor.stop();
	mInternalColorCinderTweenRef = nullptr;
}

void SpriteAnimatable::animNormalizedStop(){
	mAnimNormalized.stop();
	mInternalNormalizedCinderTweenRef = nullptr;
}

void SpriteAnimatable::completeAllTweens(const bool callFinishFunctions, const bool recursive){
	completeTweenColor(callFinishFunctions);
	completeTweenOpacity(callFinishFunctions);
	completeTweenPosition(callFinishFunctions);
	completeTweenRotation(callFinishFunctions);
	completeTweenScale(callFinishFunctions);
	completeTweenSize(callFinishFunctions);
	completeTweenNormalized(callFinishFunctions);

	if(recursive){
		const std::vector<ds::ui::Sprite*>& chillins = mOwner.getChildren();
		for(auto it = chillins.begin(), end = chillins.end(); it != end; ++it) {
			Sprite*		s(*it);
			if(s) {
				s->completeAllTweens(callFinishFunctions, true);
			}
		}
	}
}

void SpriteAnimatable::tweenAnimateOn(const bool recursive, const float delay, const float deltaDelay){
	float thisDelay = delay;
	runAnimationScript(mAnimateOnScript, thisDelay);
	if(recursive){
		const std::vector<ds::ui::Sprite*>& chillins = mOwner.getChildren();
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

void SpriteAnimatable::setAnimateOnTargetsIfNeeded(){
	if(!mAnimateOnTargetsSet){
		setAnimateOnTargets();
	}
}

void SpriteAnimatable::clearAnimateOnTargets(const bool recursive){
	mAnimateOnTargetsSet = false;
	if(recursive){
		const auto chillins = mOwner.getChildren();
		for(auto it = chillins.begin(), end = chillins.end(); it != end; ++it) {
			Sprite*		s(*it);
			if(s) {
				s->clearAnimateOnTargets(recursive);
			}
		}
	}
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
	std::map<std::string, ci::vec3> animationCommands;
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
		ci::vec3 destination = ci::vec3();

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
		ci::vec3 dest = it->second;
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
			setAnimateOnTargetsIfNeeded();
			mOwner.setPosition(mAnimateOnPositionTarget + dest);
			tweenPosition(mAnimateOnPositionTarget, dur, delayey, easing);

		} else if(animType == "fade"){
			setAnimateOnTargetsIfNeeded();
			if(dest.x == 0.0f){
				mOwner.setOpacity(0.0f);
			} else {
				mOwner.setOpacity(mAnimateOnOpacityTarget + dest.x);
			}
			tweenOpacity(mAnimateOnOpacityTarget, dur, delayey, easing);

		} else if(animType == "grow"){
			setAnimateOnTargetsIfNeeded();
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
