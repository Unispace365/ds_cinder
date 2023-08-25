#pragma once

#ifndef DS_EXAMPLE_GLOBE_UI_PIN
#define DS_EXAMPLE_GLOBE_UI_PIN

#include "cinder/CinderGlm.h"
#include "ds/ui/sprite/sprite.h"
#include "ds/ui/sprite/sprite_engine.h"
#include <cinder/CinderMath.h>

namespace ds { namespace ui {

	/*
		Geolocated Pin on a globe, use for subclassing
	*/
	class GlobePin : public Sprite {
	  public:
		GlobePin(SpriteEngine& g)
		  : Sprite(g){};
		GlobePin::~GlobePin() {}

		float getLat() { return mLat; }
		float getLong() { return mLong; }
		void  setLayer(int layer) { mLayer = layer; }
		int	  getLayer() { return mLayer; }
		void  setLongLat(float llong, float lat) {
			 mLong = llong;
			 mLat  = lat;
		}

	  private:
		float mLat;
		float mLong;
		int	  mLayer;
	};
}} // namespace ds::ui
#endif