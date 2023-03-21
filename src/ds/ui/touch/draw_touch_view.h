#pragma once
#ifndef DS_UI_TOUCH_DRAW_TOUCH_VIEW
#define DS_UI_TOUCH_DRAW_TOUCH_VIEW

#include <map>

#include <ds/cfg/settings.h>
#include <ds/data/data_buffer.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/touch/touch_manager.h>

#include <cinder/gl/Vbo.h>

namespace ds { namespace ui {
	class Circle;

	// View for drawing touches
	class DrawTouchView : public ds::ui::Sprite, public ds::ui::TouchManager::Capture {
	  public:
		DrawTouchView(ds::ui::SpriteEngine& e);
		DrawTouchView(ds::ui::SpriteEngine& e, ds::cfg::Settings& settings, ds::ui::TouchManager& tm);

		virtual void touchBegin(const ds::ui::TouchInfo& ti);
		virtual void touchMoved(const ds::ui::TouchInfo& ti);
		virtual void touchEnd(const ds::ui::TouchInfo& ti);

		virtual void drawLocalServer();

	  private:
		std::map<int, Circle*> mCircles;
		float				   mCircleRadius;
		bool				   mCircleFilled;
		ci::ColorA			   mCircleColor;
	};

}}	   // namespace ds::ui
#endif // !DS_UI_TOUCH_DRAW_TOUCH_VIEW