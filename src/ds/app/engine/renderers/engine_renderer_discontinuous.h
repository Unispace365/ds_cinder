#ifndef SRC_DS_APP_ENGINE_RENDERERS_ENGINE_DISCONTINUOUS_RENDERER_H_
#define SRC_DS_APP_ENGINE_RENDERERS_ENGINE_DISCONTINUOUS_RENDERER_H_

#include "engine_renderer_interface.h"

#include <cinder/gl/Fbo.h>

namespace ds
{

class Engine;

/*!
 * \class EngineRendererDiscontinuous
 * \namespace ds
 * \brief Experimental discontinuous renderer. Render unrelated parts of the world into one window.
 */
class EngineRendererDiscontinuous : public EngineRenderer
{
public:
	EngineRendererDiscontinuous(Engine& e);

	virtual void	drawClient() override;
	virtual void	drawServer() override;

private:
	ci::gl::FboRef		mFbo;
};

}

#endif //!SRC_DS_APP_ENGINE_RENDERERS_ENGINE_DISCONTINUOUS_RENDERER_H_