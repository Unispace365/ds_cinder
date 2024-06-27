#include "stdafx.h"

#include "split_alpha_video_player.h"

#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/util/ui_utils.h>
#include <ds/util/string_util.h>

#include <ds/ui/button/image_button.h>
#include <ds/ui/sprite/video.h>

#include "ds/ui/media/interface/video_interface.h"
#include "ds/ui/media/media_interface_builder.h"
#include "ds/ui/media/media_viewer_settings.h"

namespace ds::ui {

static std::string splitvideo_vert = R"glsl(
#version 420
// Cinder
in vec4				ciPosition;
in vec2				ciTexCoord0;
in vec4				ciColor;

// DsCinder
uniform mat4		ciModelMatrix;
uniform mat4		ciModelViewProjection;
uniform vec4		uClipPlane0;
uniform vec4		uClipPlane1;
uniform vec4		uClipPlane2;
uniform vec4		uClipPlane3;
uniform vec2		extent;
out vec4			vColor;
out vec2			TexCoord0;

void main() {
	// DsCinder / Standard
	
	gl_Position = ciModelViewProjection * ciPosition;
	TexCoord0 = ciTexCoord0;
	vColor = ciColor;

}
)glsl";

static std::string splitvideo_frag = R"glsl(
#version 420

uniform sampler2D   tex0;

in vec2  TexCoord0;
in vec4 vColor;

out vec4 fragColor;

void main() {
    fragColor = texture(tex0, vec2(TexCoord0.x,0.5+TexCoord0.y*0.5));
	vec4 alphaColor = texture(tex0, vec2(TexCoord0.x, TexCoord0.y*0.5));
	//fragColor.rgb = vec3(1.0);
	fragColor.a = alphaColor.r*vColor.a;
}
)glsl";

SplitAlphaVideoPlayer::SplitAlphaVideoPlayer(ds::ui::SpriteEngine& eng, const bool embedInterface)
  : ds::ui::VideoPlayer(eng, embedInterface) {
	setTransparent(false);
	try {
		mSplitAlphaShader = ci::gl::GlslProg::create(splitvideo_vert, splitvideo_frag);
	} catch (ci::gl::GlslProgExc e) {

		DS_LOG_INFO("GLSL EX: " << e.what());
		ci::gl::ShaderDef def;
		mSplitAlphaShader = ci::gl::getStockShader(def.color().texture());
	}
}

void SplitAlphaVideoPlayer::onUpdateServer(const ds::UpdateParams& up) {


	mVideo->setFinalRenderToTexture(true);


	mVideo->drawClient(ci::mat4(), ds::DrawParams());
	mTex = mVideo->getFinalOutTexture();
	// mVideo->setFinalRenderToTexture(false);
}

void SplitAlphaVideoPlayer::drawLocalClient() {

	if (mTex && mSplitAlphaShader) {
		mTex->bind(0);
		ci::gl::color(1.0, 1.0, 1.0, getDrawOpacity());
		// ci::gl::draw(mTex);
		mSplitAlphaShader->bind();
		ci::gl::drawSolidRect(ci::Rectf(0, 0, getWidth(), getHeight()));
		// auto size = getSize();
		// mTex->unbind();
	}
}

void SplitAlphaVideoPlayer::setResource(const ds::Resource& resource) {
	ds::ui::VideoPlayer::setResource(resource);

	setSize(mVideo->getWidth(), mVideo->getHeight() * 0.5);
	// mVideo->setPosition(0, 0);
}

void SplitAlphaVideoPlayer::onSizeChanged() {

	if (mVideo) {
		if (mVideo->getWidth() > 0.0f) {
			fitInside(mVideo, ci::Rectf(0.0f, 0.0f, getWidth(), getHeight() * 2), mLetterbox);
		}
	}
}


} // namespace ds::ui
