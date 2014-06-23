#pragma once
#ifndef DS_APP_ENGINE_ENGINEROOTS_H_
#define DS_APP_ENGINE_ENGINEROOTS_H_

#include <cinder/Camera.h>
#include "ds/app/app_defs.h"
#include "ds/cfg/settings.h"
#include "ds/ui/sprite/sprite.h"
#include "ds/params/camera_params.h"
#include "ds/params/draw_params.h"
#include "ds/params/update_params.h"
#include "ds/ui/touch/picking.h"

namespace ds {
class Engine;

/**
 * \class ds::EngineRoot
 * \brief Abstract superclass for an engine root. Pending...
 */
class EngineRoot {
public:
	static ds::ui::Sprite*		make(ui::SpriteEngine&, const ds::sprite_id_t, const bool perspective);
	EngineRoot(const RootList::Root&, const sprite_id_t);
	virtual ~EngineRoot();

	class Settings {
	public:
		Settings(	const ci::Vec2f& world_size, const ci::Rectf& screen_rect, const ds::cfg::Settings& debug_settings,
					const float default_scale, const ci::Rectf& src_rect, const ci::Rectf& dst_rect)
				: mWorldSize(world_size), mScreenRect(screen_rect), mDebugSettings(debug_settings)
				, mDefaultScale(default_scale), mSrcRect(src_rect), mDstRect(dst_rect) { }
		ci::Vec2f					mWorldSize;
		ci::Rectf					mScreenRect;
		const ds::cfg::Settings&	mDebugSettings;
		const float					mDefaultScale;
		// Obsolete scrren rect and default scale
		ci::Rectf					mSrcRect,
									mDstRect;
	};
	virtual void					setup(const Settings&) = 0;
	// Initialize myself as a slave to the master
	virtual void					slaveTo(EngineRoot*) = 0;

	const RootList::Root&			getBuilder() const;
	// Sprite management. Note that ideally Roots don't require having a sprite. Currently everything
	// does, and that was the initial design, but it would be nice to move away from that instead of
	// letting it get more entrenched.
	virtual ds::ui::Sprite*			getSprite() = 0;
	virtual void					clearChildren() = 0;
	// Sprite passthrough
	virtual void					updateClient(const ds::UpdateParams&) = 0;
	virtual void					updateServer(const ds::UpdateParams&) = 0;
	virtual void					drawClient(const DrawParams&) = 0;
	virtual void					drawServer(const DrawParams&) = 0;
	// Camera
	virtual void					markCameraDirty() = 0;
	virtual void					setCinderCamera() = 0;
	virtual ui::Sprite*				getHit(const ci::Vec3f& point) = 0;
	// Hack for manually positioning the screen.
	virtual void					setViewport(const bool) { }
	
protected:
	// The builder object for this root. Params only used during initialization.
	const RootList::Root			mRootBuilder;
	const sprite_id_t				mSpriteId;

private:
	EngineRoot(const EngineRoot&);
	EngineRoot&						operator=(const EngineRoot&);
};

/**
 * \class ds::OrthRoot
 * \brief Root for for the orthogonal camera.
 */
class OrthRoot : public EngineRoot {
public:
	OrthRoot(Engine&, const RootList::Root&, const sprite_id_t);

	virtual void					setup(const Settings&);
	virtual void					slaveTo(EngineRoot*);
	virtual ds::ui::Sprite*			getSprite();
	virtual void					clearChildren();
	virtual void					updateClient(const ds::UpdateParams&);
	virtual void					updateServer(const ds::UpdateParams&);
	virtual void					drawClient(const DrawParams&);
	virtual void					drawServer(const DrawParams&);
	virtual void					setCinderCamera();
	virtual void					setViewport(const bool b);
	virtual void					markCameraDirty();
	virtual ui::Sprite*				getHit(const ci::Vec3f& point);

private:
	void							setGlCamera();

	typedef EngineRoot				inherited;
	OrthRoot(const OrthRoot&);
	OrthRoot&						operator=(const OrthRoot&);

	Engine&							mEngine;
	ci::CameraOrtho					mCamera;
	bool							mCameraDirty;
	bool							mSetViewport;
	std::unique_ptr<ui::Sprite>		mSprite;
	// Hack in the src_rect, dst_rect stuff as I figure that out.
	ci::Rectf						mSrcRect, mDstRect;
};

/**
 * \class ds::PerspRoot
 * \brief Root for for the perspective camera.
 */
class PerspRoot : public EngineRoot {
public:
	PerspRoot(Engine&, const RootList::Root&, const sprite_id_t, const PerspCameraParams&, Picking* = nullptr);

	virtual void					setup(const Settings&);
	virtual void					slaveTo(EngineRoot*);
	virtual ds::ui::Sprite*			getSprite();
	virtual void					clearChildren();
	virtual void					updateClient(const ds::UpdateParams&);
	virtual void					updateServer(const ds::UpdateParams&);
	virtual void					drawClient(const DrawParams&);
	virtual void					drawServer(const DrawParams&);
	virtual ui::Sprite*				getHit(const ci::Vec3f& point);

	// Camera
	PerspCameraParams				getCamera() const;
	const ci::CameraPersp&			getCameraRef() const;
	void							setCamera(const PerspCameraParams&);
	virtual void					markCameraDirty();

	virtual void					setCinderCamera();

private:
	void							setGlCamera();
	void							drawFunc(const std::function<void(void)>& fn);

	typedef EngineRoot				inherited;
	PerspRoot(const PerspRoot&);
	PerspRoot&						operator=(const PerspRoot&);

	Engine&							mEngine;
	ci::CameraPersp					mCamera;
	bool							mCameraDirty;
	std::unique_ptr<ui::Sprite>		mSprite;
	// If I have a master, use it for my camera
	PerspRoot*						mMaster;

	// Maintain old and new-style picking
	class OldPick : public Picking {
	public:
		OldPick(ci::Camera&);

		virtual ds::ui::Sprite*		pickAt(const ci::Vec2f&, ds::ui::Sprite& root);

	private:
		ci::Camera&					mCamera;
	};
	OldPick							mOldPick;
	Picking&						mPicking;
};

} // namespace ds

#endif // DS_APP_ENGINE_ENGINEDATA_H_
