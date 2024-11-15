#pragma once
#ifndef DS_UI_TOUCH_DRAW_TOUCH_VIEW
#define DS_UI_TOUCH_DRAW_TOUCH_VIEW

#include <ds/cfg/settings.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/touch/touch_manager.h>

namespace ds { namespace ui {
	class Circle;

	// View for drawing touches
	class DrawTouchView : public Sprite, public TouchManager::Capture {
	  public:
		DrawTouchView(SpriteEngine& e);
		DrawTouchView(SpriteEngine& e, cfg::Settings& settings, TouchManager& tm);

		void touchBegin(const TouchInfo& ti) override;
		void touchMoved(const TouchInfo& ti) override;
		void touchEnd(const TouchInfo& ti) override;

		void drawLocalClient() override;

		void onBuildRenderBatch() override;

	  private:
		struct Instance /* std140 */ {
			ci::vec2	 position;
			glm::float32 radius;
			glm::int32_t id;

			bool operator==(const Instance& rhs) const { return id == rhs.id; }
			bool operator!=(const Instance& rhs) const { return id != rhs.id; }
			bool operator<(const Instance& rhs) const { return id < rhs.id; }

			bool operator==(const int& rhs) const { return id == rhs; }
			bool operator!=(const int& rhs) const { return id != rhs; }
			bool operator<(const int& rhs) const { return id < rhs; }
		};

		std::vector<Instance> mInstances;
		ci::gl::VboRef		  mBuffer;
		float				  mCircleRadius;
		bool				  mCircleFilled;
		ci::ColorA			  mCircleColor;
	};

}}	   // namespace ds::ui
#endif // !DS_UI_TOUCH_DRAW_TOUCH_VIEW