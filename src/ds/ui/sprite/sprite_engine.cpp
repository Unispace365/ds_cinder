<<<<<<< HEAD
#include "sprite_engine.h"
#include "sprite.h"
#include <cinder/app/App.h>
#include "ds/app/engine/engine_data.h"
#include "ds/app/engine/engine_service.h"
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"

using namespace ci;

namespace ds {
namespace ui {

/**
 * \class ds::ui::SpriteEngine
 */
SpriteEngine::SpriteEngine(ds::EngineData& ed)
	: mData(ed)
{
}

SpriteEngine::~SpriteEngine()
{
	mData.clearServices();
}

ds::EventNotifier& SpriteEngine::getNotifier()
{
	return mData.mNotifier;
}

const ds::EngineCfg& SpriteEngine::getEngineCfg() const
{
	return mData.mEngineCfg;
}

const ds::cfg::Settings& SpriteEngine::getSettings(const std::string& name) const
{
	return mData.mEngineCfg.getSettings(name);
}

float SpriteEngine::getMinTouchDistance() const
{
	return mData.mMinTouchDistance;
}

float SpriteEngine::getMinTapDistance() const
{
	return mData.mMinTapDistance;
}

unsigned SpriteEngine::getSwipeQueueSize() const
{
	return mData.mSwipeQueueSize;
}

float SpriteEngine::getDoubleTapTime() const
{
	return mData.mDoubleTapTime;
}

ci::Rectf SpriteEngine::getScreenRect() const
{
	return mData.mScreenRect;
}

float SpriteEngine::getWidth() const
{
	return mData.mScreenRect.getWidth();
}

float SpriteEngine::getHeight() const
{
	return mData.mScreenRect.getHeight();
}

float SpriteEngine::getWorldWidth() const
{
	return mData.mWorldSize.x;
}

float SpriteEngine::getWorldHeight() const
{
	return mData.mWorldSize.y;
}

void SpriteEngine::addToDragDestinationList( Sprite *sprite )
{
  if (!sprite)
    return;
  
  removeFromDragDestinationList(sprite);

  mDragDestinationSprites.push_back(sprite);
}

void SpriteEngine::removeFromDragDestinationList( Sprite *sprite )
{
  if (!sprite)
    return;

  auto found = std::find(mDragDestinationSprites.begin(), mDragDestinationSprites.end(), sprite);
  if (found != mDragDestinationSprites.end())
    mDragDestinationSprites.erase(found);
}

Sprite *SpriteEngine::getDragDestinationSprite( const ci::Vec3f &globalPoint, Sprite *draggingSprite )
{
  for (auto it = mDragDestinationSprites.begin(), it2 = mDragDestinationSprites.end(); it != it2; ++it) {
  	Sprite *sprite = *it;
    if (sprite == draggingSprite)
      continue;
    if (sprite->contains(globalPoint))
      return sprite;
  }

  return nullptr;
}

float SpriteEngine::getFrameRate() const
{
	return mData.mFrameRate;
}

std::unique_ptr<FboGeneral> SpriteEngine::getFbo()
{
  //DS_VALIDATE(width > 0 && height > 0, return nullptr);

  if (!mFbos.empty()) {
    std::unique_ptr<FboGeneral> fbo = std::move(mFbos.front());
    mFbos.pop_front();
    return std::move(fbo);
  }

  std::unique_ptr<FboGeneral> fbo = std::move(std::unique_ptr<FboGeneral>(new FboGeneral()));
  fbo->setup(true);
  return std::move(fbo);
}

void SpriteEngine::giveBackFbo( std::unique_ptr<FboGeneral> &fbo ) {
	mFbos.push_back(std::move(fbo));
}

double SpriteEngine::getElapsedTimeSeconds() const {
	return ci::app::getElapsedSeconds();
}

void SpriteEngine::clearFingers( const std::vector<int> &fingers ) {
}

ds::EngineService& SpriteEngine::private_getService(const std::string& str) {
	ds::EngineService*	s = mData.mServices[str];
	if (!s) {
		const std::string	msg = "Service (" + str + ") does not exist";
		DS_LOG_ERROR(msg);
		throw std::runtime_error(msg);
	}
	return *s;
}

} // namespace ui
} // namespace ds
=======
#include "sprite_engine.h"
#include "sprite.h"
#include <cinder/app/App.h>
#include "ds/app/engine/engine_data.h"
#include "ds/app/engine/engine_service.h"
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"

using namespace ci;

namespace ds {
namespace ui {

/**
 * \class ds::ui::SpriteEngine
 */
SpriteEngine::SpriteEngine(ds::EngineData& ed)
	: mData(ed)
{
}

SpriteEngine::~SpriteEngine()
{
	mData.clearServices();
}

ds::EventNotifier& SpriteEngine::getNotifier()
{
	return mData.mNotifier;
}

const ds::EngineCfg& SpriteEngine::getEngineCfg() const
{
	return mData.mEngineCfg;
}

const ds::cfg::Settings& SpriteEngine::getSettings(const std::string& name) const
{
	return mData.mEngineCfg.getSettings(name);
}

float SpriteEngine::getMinTouchDistance() const
{
	return mData.mMinTouchDistance;
}

float SpriteEngine::getMinTapDistance() const
{
	return mData.mMinTapDistance;
}

unsigned SpriteEngine::getSwipeQueueSize() const
{
	return mData.mSwipeQueueSize;
}

float SpriteEngine::getDoubleTapTime() const
{
	return mData.mDoubleTapTime;
}

ci::Rectf SpriteEngine::getScreenRect() const
{
	return mData.mScreenRect;
}

float SpriteEngine::getWidth() const
{
	return mData.mScreenRect.getWidth();
}

float SpriteEngine::getHeight() const
{
	return mData.mScreenRect.getHeight();
}

float SpriteEngine::getWorldWidth() const
{
	return mData.mWorldSize.x;
}

float SpriteEngine::getWorldHeight() const
{
	return mData.mWorldSize.y;
}

void SpriteEngine::addToDragDestinationList( Sprite *sprite )
{
  if (!sprite)
    return;
  
  removeFromDragDestinationList(sprite);

  mDragDestinationSprites.push_back(sprite);
}

void SpriteEngine::removeFromDragDestinationList( Sprite *sprite )
{
  if (!sprite)
    return;

  auto found = std::find(mDragDestinationSprites.begin(), mDragDestinationSprites.end(), sprite);
  if (found != mDragDestinationSprites.end())
    mDragDestinationSprites.erase(found);
}

Sprite *SpriteEngine::getDragDestinationSprite( const ci::Vec3f &globalPoint, Sprite *draggingSprite )
{
  for (auto it = mDragDestinationSprites.begin(), it2 = mDragDestinationSprites.end(); it != it2; ++it) {
  	Sprite *sprite = *it;
    if (sprite == draggingSprite)
      continue;
    if (sprite->contains(globalPoint))
      return sprite;
  }

  return nullptr;
}

float SpriteEngine::getFrameRate() const
{
	return mData.mFrameRate;
}

std::unique_ptr<FboGeneral> SpriteEngine::getFbo()
{
  //DS_VALIDATE(width > 0 && height > 0, return nullptr);

  if (!mFbos.empty()) {
    std::unique_ptr<FboGeneral> fbo = std::move(mFbos.front());
    mFbos.pop_front();
    return std::move(fbo);
  }

  std::unique_ptr<FboGeneral> fbo = std::move(std::unique_ptr<FboGeneral>(new FboGeneral()));
  fbo->setup(true);
  return std::move(fbo);
}

void SpriteEngine::giveBackFbo( std::unique_ptr<FboGeneral> &fbo ) {
	mFbos.push_back(std::move(fbo));
}

double SpriteEngine::getElapsedTimeSeconds() const {
	return ci::app::getElapsedSeconds();
}

void SpriteEngine::clearFingers( const std::vector<int> &fingers ) {
}

ds::EngineService& SpriteEngine::private_getService(const std::string& str) {
	ds::EngineService*	s = mData.mServices[str];
	if (!s) {
		const std::string	msg = "Service (" + str + ") does not exist";
		DS_LOG_ERROR(msg);
		throw std::runtime_error(msg);
	}
	return *s;
}

} // namespace ui
} // namespace ds
>>>>>>> origin/master
