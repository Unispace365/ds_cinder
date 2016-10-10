#ifndef SRC_DS_APP_ENGINE_RENDERERS_ENGINE_CONTINUOUS_RENDERER_FXAA_H_
#define SRC_DS_APP_ENGINE_RENDERERS_ENGINE_CONTINUOUS_RENDERER_FXAA_H_

#include "engine_renderer_interface.h"

#include <cinder/Rect.h>
#include <cinder/gl/Fbo.h>
#include <cinder/gl/GlslProg.h>

namespace ds
{

class Engine;

/*!
 * \class EngineRendererContinuousFxaa
 * \namespace ds
 * \brief An expansion to Generic renderer to support rendering with FXAA
 * \see http://en.wikipedia.org/wiki/Fast_approximate_anti-aliasing
 */
class EngineRendererContinuousFxaa final : public EngineRenderer
{
public:
	EngineRendererContinuousFxaa(Engine& e);

	virtual void		drawClient() override;
	virtual void		drawServer() override;

private:
	ci::Rectf			mRenderRect;
	ci::gl::Fbo			mFbo;
	ci::gl::GlslProg	mFxaaShader;
	ci::vec2			mTexCoordOffset;
};

}

#endif //!SRC_DS_APP_ENGINE_RENDERERS_ENGINE_CONTINUOUS_RENDERER_FXAA_H_