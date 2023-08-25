
#include "ds/math/math_defs.h"
#include "ds/params/draw_params.h"


#include "../globe_view.h"
#include "globe_pin.h"
#include "globe_pin_manager.h"


#include <math.h>

namespace ds { namespace ui {
	namespace {

		// flags
		const int VISIBLE_F			= (1 << 0);
		const int TRANSPARENT_F		= (1 << 1);
		const int ENABLED_F			= (1 << 2);
		const int DRAW_SORTED_F		= (1 << 3);
		const int CLIP_F			= (1 << 4);
		const int SHADER_CHILDREN_F = (1 << 5);
		const int NO_REPLICATION_F	= (1 << 6);
		const int ROTATE_TOUCHES_F	= (1 << 7);

	} // namespace

	GlobePinManager::GlobePinManager(SpriteEngine& se, float updateInterval)
	  : Sprite(se)
	  , mEngine(se)
	  , mElapsedTime(0.0f)
	  , mUpdateInterval(updateInterval)
	  , mGlobeRadius(400)
	  , mXrotation(0)
	  , mYrotation(0) {}

	GlobePinManager::~GlobePinManager() {}

	/*
		Add a pin to a given layer. If no layer is specified add it to the default layer
	*/
	void GlobePinManager::addPin(GlobePin* pin) {
		// If [] finds nothing it creates a new vector so this is safe
		if (pin != nullptr) {
			addChildPtr(pin);
			mPinLayers[pin->getLayer()].push_back(pin);
		}
	}

	void GlobePinManager::clearFilters() {
		for (auto f : mActiveFilters) {
			filterUpdated(f, false);
		}
		mActiveFilters.clear();
	}

	void GlobePinManager::addFilter(int nfilt) {
		mActiveFilters.push_back(nfilt);
		filterUpdated(nfilt, true);
	}

	void GlobePinManager::removeFilter(int nfilt) {
		for (auto it = mActiveFilters.begin(); it != mActiveFilters.end(); ++it) {
			filterUpdated(nfilt, false);
			mActiveFilters.erase(it);
		}
	}

	void GlobePinManager::filterUpdated(int filt, bool show) {
		if (mPinLayers[filt].empty()) return;
		for (auto v : mPinLayers[filt]) {
			(show) ? v->show() : v->hide();
		}
	}

	void GlobePinManager::updateRotation(float xRot, float yRot) {
		mXrotation = xRot;
		mYrotation = yRot;

		for (auto pl : mPinLayers) {
			for (auto pin : pl.second) {

				auto v = mGlobeRadius * longLat2WorldCoords(mLongOffset + pin->getLong(), mLatOffset + pin->getLat());

				v = glm::rotateY(v, ci::toRadians(yRot));
				v = glm::rotateX(v, ci::toRadians(xRot));

				auto mCam = mEngine.getPerspectiveCameraRef(0);
				auto p	  = mCam.worldToScreen(v, mEngine.getWorldWidth(), mEngine.getWorldHeight());
				pin->setPosition(v);


				// Normalize our depth 1.0 being closest, 0 being furthest
				float depth = ((v.z + (mGlobeRadius) / 2.0) / mGlobeRadius);
				depth		= ci::smoothstep(0.60f, 0.70f, depth);
				pin->setOpacity(depth);
				pin->setScale(depth);
			}
		}
	}

	float GlobePinManager::longLatDist(float p1_long, float p1_lat, float p2_long, float p2_lat) {
		float earthRadius = 3961.0f; // miles
		float p1_lat_rad  = p1_lat * math::DEGREE2RADIAN;
		float p2_lat_rad  = p2_lat * math::DEGREE2RADIAN;

		float p1_lng_rad = p1_long * math::DEGREE2RADIAN;
		float p2_lng_rad = p2_long * math::DEGREE2RADIAN;


		// float dLon = p2_long* math::DEGREE2RADIAN - p1_long*math::DEGREE2RADIAN;
		// float dLat = p2_lat* math::DEGREE2RADIAN - p1_lat* math::DEGREE2RADIAN;
		float dLon = p2_lng_rad - p1_lng_rad;
		float dLat = p2_lat_rad - p1_lat_rad;

		float a = pow(sin(dLat / 2.0f), 2.0f);
		a += cos(p1_lat_rad) * cos(p2_lat_rad) * pow(sin(dLon / 2.0f), 2.0f);

		float c = 2.0f * atan2(sqrt(a), sqrt(1.0f - a));
		float d = earthRadius * c;
		return d;
	}

	// Determine where in the world the pins should be positioned.
	ci::vec3 GlobePinManager::longLat2WorldCoords(float longitude, float latitude) {
		float lat = latitude * math::DEGREE2RADIAN;
		float lon = longitude * math::DEGREE2RADIAN;

		ci::vec3 xyz;

		xyz.z = cos(lat) * cos(lon);
		xyz.x = cos(lat) * sin(lon);
		xyz.y = sin(lat);

		return xyz;
	}


}} // namespace ds::ui
