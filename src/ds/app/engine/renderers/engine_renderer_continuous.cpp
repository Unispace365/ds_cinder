#include "engine_renderer_continuous.h"

#include <ds/app/engine/engine.h>
#include <ds/app/engine/engine_roots.h>

#include <cinder/gl/gl.h>

namespace ds
{

EngineRendererContinuous::EngineRendererContinuous(Engine& e)
	: EngineRenderer(e)
{}

void EngineRendererContinuous::drawClient()
{
	ci::gl::enableAlphaBlending();
	
	clearScreen();

	for (auto it = mEngine.getRoots().begin(), end = mEngine.getRoots().end(); it != end; ++it)
	{
		(*it)->drawClient(mEngine.getDrawParams(), mEngine.getAutoDrawService());
	}
}

void EngineRendererContinuous::drawServer()
{
	// TODO ?
	//glAlphaFunc(GL_GREATER, 0.001f);

	//ci::gl::enable(GL_ALPHA_TEST);
	ci::gl::enableAlphaBlending();
	clearScreen();

	for (auto it = mEngine.getRoots().cbegin(), end = mEngine.getRoots().cend(); it != end; ++it)
	{
		(*it)->drawServer(mEngine.getDrawParams());
	}

	//glAlphaFunc(GL_ALWAYS, 0.001f);
}

}