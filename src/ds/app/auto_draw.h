#pragma once
#ifndef DS_APP_AUTODRAW_H_
#define DS_APP_AUTODRAW_H_

#include <vector>

#include <cinder/Matrix.h>

#include "ds/app/engine/engine_service.h"

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
	AutoDraw(ui::SpriteEngine&);
	virtual ~AutoDraw();

	AutoDraw(const AutoDraw&)			 = delete;
	AutoDraw(AutoDraw&&)				 = delete;
	AutoDraw& operator=(const AutoDraw&) = delete;
	AutoDraw& operator=(AutoDraw&&)		 = delete;

  protected:
	virtual void drawClient(const ci::mat4&, const DrawParams&) = 0;

  private:
	friend class AutoDrawService;

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
