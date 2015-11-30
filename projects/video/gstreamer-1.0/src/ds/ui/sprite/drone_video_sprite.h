#ifndef DELOITTE_PRESENTATION_SRC_DRONE_VIDEO_SPRITE_H_
#define DELOITTE_PRESENTATION_SRC_DRONE_VIDEO_SPRITE_H_

#include <ds/ui/sprite/sprite.h>

#include <cinder/gl/Texture.h>
#include <cinder/gl/GlslProg.h>
#include <cinder/Camera.h>
#include <cinder/Sphere.h>
#include <ds/ui/sprite/util/configurable.h>

#include "ds/ui/sprite/video.h"


namespace ds {
	class Engine;
	namespace ui {

class DroneVideoSprite final
	: public ds::ui::Sprite
	, private ds::util::Configurable
{

public:
	DroneVideoSprite(ds::ui::SpriteEngine&);

protected:
	// These are our only chances in client mode to catch the video.
	virtual void onChildAdded(ds::ui::Sprite& child) override;
	virtual void onChildRemoved(ds::ui::Sprite& child) override;
	virtual void onSizeChanged() override;
	virtual void updateServer(const ds::UpdateParams&) override;
	virtual void updateClient(const ds::UpdateParams&) override;
	virtual void drawLocalClient() override;
	virtual void onRotationChanged() override;
	virtual void handleDrag(const ci::Vec2f&);

public:
	// API:
	void				installVideo(const std::string&);
	void				installVideo(ds::ui::Video* const video, const std::string& path);
	ds::ui::Video*		getVideo() const;
	void				resetCamera();

	void				lookFront();
	void				lookBack();
	void				lookUp();
	void				lookDown();
	void				lookRight();
	void				lookLeft();

	// set the spherical coordinate (in degrees).
	/*********************************
	(C) for ASCII art! Sepehr Laal.

			+ z (UP)
			 |
			 |
			 |-+
			 |  \
			 +-- \ ------ + y (RIGHT)
			/ \  theta
		   /   \  |
		  /     \
		 / -phi- \
	   + x
	 (FRONT)
	*********************************/
	void				setSphericalCoord(float theta, float phi);

	float				getTheta() const { return mSphericalAngles.x; }
	float				getPhi() const { return mSphericalAngles.y; }

private:
	ds::ui::Video*		mVideoSprite;
	ci::gl::Texture*	mVideoTexture;
	ci::gl::GlslProgRef	mShader;
	ci::CameraPersp     mCamera;
	ci::Sphere          mSphere;
	ci::Vec2f			mSphericalAngles; // x is theta and y is phi

public:
	static void			installAsServer(ds::BlobRegistry&);
	static void			installAsClient(ds::BlobRegistry&);
	static void			installSprite(ds::Engine&);

};

}} //!dlpr::view

#endif //!DELOITTE_PRESENTATION_SRC_DRONE_VIDEO_SPRITE_H_
