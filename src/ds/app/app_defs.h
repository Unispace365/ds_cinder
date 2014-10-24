#pragma once
#ifndef DS_APP_APPDEFS_H_
#define DS_APP_APPDEFS_H_

#include <functional>
#include <vector>
#include "ds/params/camera_params.h"

namespace ds {

typedef int                 sprite_id_t;
// System-defined illegal sprite
static const sprite_id_t    EMPTY_SPRITE_ID = 0;

// Blob termination for network traffic
static const char           TERMINATOR_CHAR = 0;

// Types of auto updates
namespace AutoUpdateType {
	static const int		SERVER = (1<<0);
	static const int		CLIENT = (1<<1);
}

/**
 * \class ds::RootList
 * \brief Used during the app constructor to supply a list
 * of roots and any associated params. Example use:
 * AppSubclass() : App(ds::RootList().ortho().persp().ortho())
 * or
 * AppSubclass() : App(ds::RootList().ortho().persp().ortho())
 */
class RootList {
public:
	// Not explicit on purpose. Provide backwards compatibilty
	// for the old way of specifying roots.
	RootList(const std::vector<int>* roots = nullptr);
	// With this variant, the function is called before the
	// roots are installed in the engine, which occurs after
	// some of the engine setup (like settings).
	RootList(const std::function<RootList(void)>&);

	bool							empty() const;
	// Answer the result of running my init fn, if I have one,
	// or just a copy of me.
	RootList						runInitFn() const;

	// ADD ROOT TYPES.
	RootList&						ortho();
	RootList&						persp();

	// SET ROOT PARAMETERS. All of these apply to the currently active root.
	// Use OpenGL SELECT for picking.
	RootList&						pickSelect();
	// Use unique colour rendering for picking.
	RootList&						pickColor();
	
	RootList&						perspFov(const float);
	RootList&						perspPosition(const ci::Vec3f&);
	RootList&						perspTarget(const ci::Vec3f&);
	RootList&						perspNear(const float);
	RootList&						perspFar(const float);

	// Set to master or slave mode. Currently only perspectives can
	// be master or slave, there can only be a single master, and all
	// slaves will follow the master's camera settings.
	RootList&						master();
	RootList&						slave();

	class Root {
	public:
		Root();

		enum Type					{ kOrtho, kPerspective };
		Type						mType;
		enum Pick					{ kDefault, kSelect, kColor };
		Pick						mPick;
		enum Master					{ kIndependent, kMaster, kSlave };
		Master						mMaster;
		PerspCameraParams			mPersp;
		
		bool						mDebugDraw;
	};

private:
	friend class Engine;
	std::vector<Root>				mRoots;
	// This allows clients to initialize the root list after the
	// engine has been (mostly) constructed.
	std::function<RootList(void)>	mInitFn;
};

} // namespace ds

#endif // DS_APP_APPDEFS_H_