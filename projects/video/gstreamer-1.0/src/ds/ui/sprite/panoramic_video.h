#pragma once

#include <ds/ui/sprite/sprite.h>

#include <cinder/Camera.h>
#include <cinder/Sphere.h>
#include <cinder/gl/GlslProg.h>
#include <cinder/gl/Texture.h>

#include "ds/ui/sprite/video.h"


namespace ds {
class Engine;
namespace ui {

	class PanoramicVideo : public ds::ui::Sprite {

	  public:
		PanoramicVideo(ds::ui::SpriteEngine&, const bool textureInvertX = false);

		virtual void   setResource(const ds::Resource& resource) override;
		void		   loadVideo(const std::string& videoPath);
		ds::ui::Video* getVideo() const;
		void		   resetCamera();
		void		   setCameraRotation(ci::vec2 setter);
		ci::vec2	   getCameraRotation();


		// Sets how fast dragging around is. Higher numbers are slower panning, lower numbers are faster
		// Default = 5.0f
		void setDragParams(const float xSensitivity, const float ySensitivity);

		// Sets the drag direction, default x=false, y=true
		void setDragInvert(const bool xInvert, const bool yInvert);

		void handleDrag(const ci::vec2& deltaPos);

		void setFOV(const float fov);

		// Applied to video when video is loaded
		void setPan(const float audioPan);
		void setPlayableInstances(const std::vector<std::string> instances);
		void setAutoSyncronize(const bool doSync); // synchronize across client/server

		void setPanoTappedCallback(std::function<void(void)> cb) { mPanoTappedCb = cb; }

	  protected:
		// These are our only chances in client mode to catch the video.
		virtual void onChildAdded(ds::ui::Sprite& child) override;
		virtual void onChildRemoved(ds::ui::Sprite& child) override;
		virtual void onSizeChanged() override;
		virtual void drawLocalClient() override;
		virtual void writeAttributesTo(ds::DataBuffer&);
		virtual void readAttributeFrom(const char attributeId, ds::DataBuffer&);

		virtual void onBuildRenderBatch() override;

	  private:
		ds::ui::Video* mVideoSprite;

		ci::CameraPersp	   mCamera;
		ci::gl::VboMeshRef mSphereVbo;

		bool  mInvertX;
		bool  mInvertY;
		bool  mTexInvertX;
		float mXSensitivity;
		float mYSensitivity;

		float mXRot;
		float mYRot;
		float mFov;

		float					 mPanning;
		std::vector<std::string> mPlayableInstances;
		bool					 mAutoSync;
		std::string				 mSphereVertexShader;
		std::string				 mSphereFragmentShader;
		std::string				 mSphereFragmentShaderInvertX;
		ci::gl::GlslProgRef		 mShader;

		std::function<void(void)> mPanoTappedCb = nullptr;

	  public:
		static void installAsServer(ds::BlobRegistry&);
		static void installAsClient(ds::BlobRegistry&);
		static void installSprite(ds::Engine&);
	};

} // namespace ui
} // namespace ds
