#include "engine_renderer_continuous_fxaa.h"

#include <ds/app/engine/engine.h>
#include <ds/app/environment.h>
#include <ds/app/engine/engine_roots.h>

#include <cinder/gl/gl.h>

namespace ds
{

EngineRendererContinuousFxaa::EngineRendererContinuousFxaa(Engine& e)
	: EngineRenderer(e)
	, mRenderRect(0.0f, e.getHeight(), e.getWidth(), 0.0f)
	, mTexCoordOffset(1.0f / mEngine.getWidth(), 1.0f / mEngine.getHeight())
{
	ci::gl::Fbo::Format fmt;
	fmt.setColorInternalFormat(GL_RGBA32F);

	const auto w = static_cast<int>(e.getWidth());
	const auto h = static_cast<int>(e.getHeight());

	mFbo = ci::gl::Fbo(w, h, fmt);

	std::string location = ds::Environment::getAppFolder("data/shaders");
	std::string name = "fxaa";
	try {
		mFxaaShader = ci::gl::GlslProg(ci::loadFile((location + "/" + name + ".vert").c_str()), ci::loadFile((location + "/" + name + ".frag").c_str()));
	}
	catch (std::exception &e) {
		std::cout << e.what() << std::endl;
	}
}

void EngineRendererContinuousFxaa::drawClient()
{
	glAlphaFunc(GL_GREATER, 0.001f);
	ci::gl::enable(GL_ALPHA_TEST);

	{
		ci::gl::SaveFramebufferBinding bindingSaver(mFbo);
		//mFbo.bindFramebuffer();
		
		ci::gl::enableAlphaBlending();
		clearScreen();
		for (auto it = mEngine.getRoots().cbegin(), end = mEngine.getRoots().cend(); it != end; ++it) {
			(*it)->drawClient(mEngine.getDrawParams(), mEngine.getAutoDrawService());
		}

		mFbo.unbindFramebuffer();
	}

	ci::gl::enableAlphaBlending();
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	clearScreen();

	if (mFxaaShader)
	{
		mFxaaShader.bind();
		mFbo.bindTexture();
		mFxaaShader.uniform("tex0", 0);
		mFxaaShader.uniform("texcoordOffset", mTexCoordOffset);
		mFxaaShader.uniform("FXAA_SPAN_MAX", mEngine.getFxaaOptions().mFxAASpanMax);
		mFxaaShader.uniform("FXAA_REDUCE_MUL", 1.0f / mEngine.getFxaaOptions().mFxAAReduceMul);
		mFxaaShader.uniform("FXAA_REDUCE_MIN", 1.0f / mEngine.getFxaaOptions().mFxAAReduceMin);

		ci::gl::drawSolidRect(mRenderRect);

		mFbo.unbindTexture();
		mFxaaShader.unbind();
	}
	else {
		ci::gl::draw(mFbo.getTexture(0), mRenderRect);
	}

	glAlphaFunc(GL_ALWAYS, 0.001f);
}

void EngineRendererContinuousFxaa::drawServer()
{
	glAlphaFunc(GL_GREATER, 0.001f);

	ci::gl::enable(GL_ALPHA_TEST);
	ci::gl::enableAlphaBlending();
	clearScreen();

	for (auto it = mEngine.getRoots().cbegin(), end = mEngine.getRoots().cend(); it != end; ++it)
	{
		(*it)->drawServer(mEngine.getDrawParams());
	}

	glAlphaFunc(GL_ALWAYS, 0.001f);
}

}