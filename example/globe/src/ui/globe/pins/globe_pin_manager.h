#pragma once
#ifndef DS_EXAMPLE_GLOBE_UI_LOCATOR_MANAGER
#define DS_EXAMPLE_GLOBE_UI_LOCATOR_MANAGER

#include "cinder/json.h"
#include "ds/ui/sprite/sprite.h"
#include "ds/ui/sprite/sprite_engine.h"

namespace ds { namespace ui {

	class GlobePin;
	class GlobeView;

	class GlobePinManager : public Sprite {
	  public:
		GlobePinManager(SpriteEngine& se, float updateInterval = 10);
		GlobePinManager::~GlobePinManager();

		void updateRotation(float xRot, float yRot);
		void updateRadius(float newRadius) { mGlobeRadius = newRadius; }

		void checkForUpdates();

		void  resetElapsedTime() { mElapsedTime = 0; }
		float getXrotation() { return mXrotation; }
		float getYrotation() { return mYrotation; }
		void  makeSortedChildren();

		float getLongitudeOffset() { return mLongOffset; }
		float getLatitudeOffset() { return mLatOffset; }

		void clearFilters();
		void addFilter(int nfilt);
		void removeFilter(int nfilt);

		void addPin(GlobePin* pin);

	  private:
		void filterUpdated(int filt, bool show);

		float longLatDist(float p1_long, float p1_lat, float p2_long, float p2_lat);

		SpriteEngine& mEngine;
		ci::vec3	  longLat2WorldCoords(float longitude, float latitude);
		float		  mElapsedTime;
		float		  mUpdateInterval;

		float mXrotation;
		float mYrotation;

		float mGlobeRadius;
		// Calibrate to latitude=0 & longitude=0 in real life.
		const float mLongOffset = 180;
		const float mLatOffset	= 0.0f;

		// Used for depth sorting;
		std::vector<Sprite*> mSortedTmp;
		// Holds our pin sprites, allows filtering

		std::vector<int>										mActiveFilters;
		std::unordered_map<int, std::vector<ds::ui::GlobePin*>> mPinLayers;
	};
}} // namespace ds::ui
#endif