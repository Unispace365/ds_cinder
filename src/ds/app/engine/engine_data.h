#pragma once
#ifndef DS_APP_ENGINE_ENGINEDATA_H_
#define DS_APP_ENGINE_ENGINEDATA_H_

#include <unordered_map>

#include <cinder/Rect.h>

#include "ds/app/engine/engine_cfg.h"
#include "ds/app/event_notifier.h"

namespace ds {
class EngineService;

/**
 * \class EngineData
 * \brief Store all the data for an engine. Primarily a
 * programmer convenience, to hide and group info.
 */
class EngineData {
  public:
	EngineData(cfg::Settings& engine_settings);
	~EngineData() = default;

	EngineData(const EngineData&)			 = delete;
	EngineData(EngineData&&)				 = delete;
	EngineData& operator=(const EngineData&) = delete;
	EngineData& operator=(EngineData&&)		 = delete;

	EventNotifier									mNotifier;
	std::unordered_map<std::string, EngineService*> mServices;

	/// Will stop and delete them.
	void clearServices();

	EngineCfg mEngineCfg;

	float		mMinTouchDistance;
	float		mMinTapDistance;
	int			mSwipeQueueSize;
	float		mSwipeMinVelocity;
	float		mSwipeMaxTime;
	float		mDoubleTapTime;
	ci::vec2	mWorldSize;
	float		mFrameRate;
	int			mIdleTimeout;
	std::string mAppInstanceName;
	float		mAnimDur;
	std::string mCmsURL;

	/// The source rect in world bounds and the destination
	/// local rect.
	ci::Rectf mSrcRect, mDstRect;
	/// the srcRect loaded from settings (for restoring after manual translation)
	ci::Rectf mOriginalSrcRect;

	/// Volume control for the whole app

	bool mMute;
};

} // namespace ds

#endif // DS_APP_ENGINE_ENGINEDATA_H_
