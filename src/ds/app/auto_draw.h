#pragma once
#ifndef DS_APP_AUTODRAW_H_
#define DS_APP_AUTODRAW_H_

#include <vector>
#include <cinder/MatrixAffine2.h>
#include <cinder/Matrix22.h>
#include <cinder/Matrix33.h>
#include <cinder/Matrix44.h>
#include "ds/app/engine/engine_service.h"

namespace ds {
class DrawParams;
class AutoDrawService;
namespace ui {
class SpriteEngine;
}

/**
 * \class ds::AutoDraw
 * \brief Utility to let any class participate in drawing.
 */
class AutoDraw {
public:
	AutoDraw(ds::ui::SpriteEngine&);
	virtual ~AutoDraw();

protected:
	virtual void		drawClient(const ci::Matrix44f&, const DrawParams&) = 0;

private:
	friend class AutoDrawService;
	AutoDraw();

	AutoDrawService&	mOwner;
};

/**
 * \class ds::AutoDrawService
 * Store a collection of auto draw objects.
 */
class AutoDrawService : public EngineService {
public:
    AutoDrawService();

	virtual void			drawClient(const ci::Matrix44f&, const DrawParams&);

private:
	friend class AutoDraw;
	std::vector<AutoDraw*>	mUpdate;
};

} // namespace ds

#endif // DS_APP_AUTOUPDATE_H_