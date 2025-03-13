#pragma once
#ifndef DS_APP_ENGINE_ENGINEROOTS_H_
#define DS_APP_ENGINE_ENGINEROOTS_H_

#include <cinder/Camera.h>

#include "ds/app/app_defs.h"
#include "ds/cfg/settings.h"
#include "ds/params/camera_params.h"
#include "ds/params/draw_params.h"
#include "ds/ui/sprite/sprite.h"
#include "ds/ui/touch/picking.h"

namespace ds {
class AutoDrawService;
class Engine;

/**
 * \class EngineRoot
 * \brief Abstract superclass for an engine root. Pending...
 */
class EngineRoot {
  public:
	static ui::Sprite* make(ui::SpriteEngine&, sprite_id_t, bool perspective);
	EngineRoot(const RootList::Root&, sprite_id_t);
	virtual ~EngineRoot() = default;

	EngineRoot(const EngineRoot&)			 = delete;
	EngineRoot(EngineRoot&&)				 = delete;
	EngineRoot& operator=(const EngineRoot&) = delete;
	EngineRoot& operator=(EngineRoot&&)		 = delete;

	class Settings {
	  public:
		Settings(const ci::vec2& world_size, const ci::Rectf& src_rect, const ci::Rectf& dst_rect)
		  : mWorldSize(world_size)
		  , mSrcRect(src_rect)
		  , mDstRect(dst_rect) {}
		ci::vec2  mWorldSize;
		ci::Rectf mSrcRect, mDstRect;
	};
	virtual void setup(const Settings&) = 0;
	virtual void postAppSetup()			= 0;
	/// Initialize myself as a slave to the master
	virtual void slaveTo(EngineRoot*) = 0;

	const RootList::Root& getBuilder() const;
	/// Sprite management. Note that ideally Roots don't require having a sprite. Currently everything
	/// does, and that was the initial design, but it would be nice to move away from that instead of
	/// letting it get more entrenched.
	virtual ui::Sprite* getSprite()		= 0;
	virtual void		clearChildren() = 0;
	/// Sprite pass-through
	virtual void updateClient(const UpdateParams&)				 = 0;
	virtual void updateServer(const UpdateParams&)				 = 0;
	virtual void drawClient(const DrawParams&, AutoDrawService*) = 0;
	virtual void drawServer(const DrawParams&)					 = 0;
	/// Camera
	virtual void		markCameraDirty()			  = 0;
	virtual void		setCinderCamera()			  = 0;
	virtual ui::Sprite* getHit(const ci::vec3& point) = 0;

  protected:
	/// The builder object for this root. Params only used during initialization.
	const RootList::Root mRootBuilder;
	const sprite_id_t	 mSpriteId;
};

/**
 * \class OrthRoot
 * \brief Root for for the orthogonal camera.
 */
class OrthRoot : public EngineRoot {
  public:
	OrthRoot(Engine&, const RootList::Root&, sprite_id_t);

	OrthRoot(const OrthRoot&)			 = delete;
	OrthRoot(OrthRoot&&)				 = delete;
	OrthRoot& operator=(const OrthRoot&) = delete;
	OrthRoot& operator=(OrthRoot&&)		 = delete;

	void		setup(const Settings&) override;
	void		postAppSetup() override;
	void		slaveTo(EngineRoot*) override;
	ui::Sprite* getSprite() override;
	void		clearChildren() override;
	void		updateClient(const UpdateParams&) override;
	void		updateServer(const UpdateParams&) override;
	void		drawClient(const DrawParams&, AutoDrawService*) override;
	void		drawServer(const DrawParams&) override;
	void		setCinderCamera() override;
	void		markCameraDirty() override;
	ui::Sprite* getHit(const ci::vec3& point) override;

	float getNearPlane() const { return mNearPlane; };
	float getFarPlane() const { return mFarPlane; };
	void  setViewPlanes(const float nearPlane, const float farPlane) {
		 mNearPlane = nearPlane;
		 mFarPlane	= farPlane;
	}

  private:
	void setGlCamera();

	Engine&						mEngine;
	ci::CameraOrtho				mCamera;
	bool						mCameraDirty;
	std::unique_ptr<ui::Sprite> mSprite;
	/// Hack in the src_rect, dst_rect stuff as I figure that out.
	ci::Rectf mSrcRect, mDstRect;

	/// The drawing distance near and far, default = -1 and 1
	float mNearPlane;
	float mFarPlane;
};

/**
 * \class PerspRoot
 * \brief Root for for the perspective camera.
 */
class PerspRoot : public EngineRoot {
  public:
	PerspRoot(Engine&, const RootList::Root&, const sprite_id_t, const PerspCameraParams&);

	PerspRoot(const PerspRoot&)			   = delete;
	PerspRoot(PerspRoot&&)				   = delete;
	PerspRoot& operator=(const PerspRoot&) = delete;
	PerspRoot& operator=(PerspRoot&&)	   = delete;

	void		setup(const Settings&) override;
	void		postAppSetup() override;
	void		slaveTo(EngineRoot*) override;
	ui::Sprite* getSprite() override;
	void		clearChildren() override;
	void		updateClient(const UpdateParams&) override;
	void		updateServer(const UpdateParams&) override;
	void		drawClient(const DrawParams&, AutoDrawService*) override;
	void		drawServer(const DrawParams&) override;
	ui::Sprite* getHit(const ci::vec3& point) override;

	/// Camera
	PerspCameraParams getCamera() const;
	void			  setCamera(const PerspCameraParams&);

	const ci::CameraPersp& getCameraRef() const;
	void				   setCameraRef(const ci::CameraPersp&);

	void markCameraDirty() override;

	void setCinderCamera() override;

	// Moved from private to here in order to update camera parameters on the fly at draw time.
	void setGlCamera() const;


  protected:
	PerspCameraParams mCameraParams;

  private:
	void drawFunc(const std::function<void(void)>& fn);

	Engine&						mEngine;
	ci::CameraPersp				mCamera;
	bool						mCameraDirty;
	std::unique_ptr<ui::Sprite> mSprite;
	/// If I have a master, use it for my camera
	PerspRoot* mMaster;
};

} // namespace ds

#endif // DS_APP_ENGINE_ENGINEDATA_H_
