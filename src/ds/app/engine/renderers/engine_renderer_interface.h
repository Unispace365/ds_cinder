#ifndef SRC_DS_APP_ENGINE_RENDERERS_ENGINE_INTERFACE_RENDERER_H_
#define SRC_DS_APP_ENGINE_RENDERERS_ENGINE_INTERFACE_RENDERER_H_

namespace ds
{

class Engine;

/*!
 * \class EngineRenderer
 * \namespace ds
 * \brief An abstract interface for different engine renderers.
 */
class EngineRenderer
{
	//! We do not want to be copied, nor moved.
	EngineRenderer()						= delete;
	EngineRenderer(const EngineRenderer&)	= delete;
	EngineRenderer(EngineRenderer&&)		= delete;

public:
	//! Renderer keeps a reference to Engine for data
	EngineRenderer(Engine& engine) : mEngine(engine) {}
	virtual ~EngineRenderer() {}

	//! Draws client (mostly used, in CLIENT, CLIENTSERVER, and STANDALONE mode)
	virtual void		drawClient() = 0;
	//! Draws server (rarely used, in SERVER mode)
	virtual void		drawServer() = 0;
	//! Clears screen
	virtual void		clearScreen();

protected:
	Engine&				mEngine;
};

}

#endif //!SRC_DS_APP_ENGINE_RENDERERS_ENGINE_INTERFACE_RENDERER_H_