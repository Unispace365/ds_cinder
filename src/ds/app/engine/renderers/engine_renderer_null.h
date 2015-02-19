#ifndef SRC_DS_APP_ENGINE_RENDERERS_ENGINE_NULL_RENDERER_H_
#define SRC_DS_APP_ENGINE_RENDERERS_ENGINE_NULL_RENDERER_H_

#include "engine_renderer_interface.h"

namespace ds
{

class Engine;

/*!
 * \class EngineRendererNull
 * \namespace ds
 * \brief Null renderer. Does not render anything. Suitable for stress testing.
 * \note activate by setting "null_renderer" bool entry in engine.xml
 */
class EngineRendererNull final : public EngineRenderer
{
public:
	EngineRendererNull(Engine& e) : EngineRenderer(e) {}

	virtual void drawClient() override { /*no op*/ }
	virtual void drawServer() override { /*no op*/ }
};

}

#endif //!SRC_DS_APP_ENGINE_RENDERERS_ENGINE_NULL_RENDERER_H_