#pragma once
#ifndef DS_APP_AUTODRAW_H_
#define DS_APP_AUTODRAW_H_

#include "cinder/gl/gl.h"
#include "ds/app/engine/engine_service.h"
#include <vector>

namespace ds {
class DrawParams;
class AutoDrawService;
namespace ui {
	class SpriteEngine;
}

/**
 * \class AutoDraw
 * \brief Utility to let any class participate in drawing.
 */
class AutoDraw {
  public:
	AutoDraw(ds::ui::SpriteEngine&);
	virtual ~AutoDraw();

  protected:
	virtual void drawClient(const ci::mat4&, const DrawParams&) = 0;

  private:
	friend class AutoDrawService;
	AutoDraw();

	AutoDrawService& mOwner;
};

/**
 * \class AutoDrawService
 * Store a collection of auto draw objects.
 */
class AutoDrawService : public EngineService {
  public:
	AutoDrawService();

	virtual void drawClient(const ci::mat4&, const DrawParams&);

  private:
	friend class AutoDraw;
	std::vector<AutoDraw*> mUpdate;
};

} // namespace ds

#endif
