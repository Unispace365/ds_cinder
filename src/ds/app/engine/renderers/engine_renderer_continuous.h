#ifndef SRC_DS_APP_ENGINE_RENDERERS_ENGINE_CONTINUOUS_RENDERER_H_
#define SRC_DS_APP_ENGINE_RENDERERS_ENGINE_CONTINUOUS_RENDERER_H_

#include "engine_renderer_interface.h"

namespace ds
{

class Engine;

/*!
 * \class EngineRendererContinuous
 * \namespace ds
 * \brief Generic renderer of DS Cinder. Render everything with no special effects.
 * \note supports ONE src_rect and ONE dst_rect only.
 */
class EngineRendererContinuous final : public EngineRenderer
{
public:
	EngineRendererContinuous(Engine& e);

	virtual void drawClient() override;
	virtual void drawServer() override;
};

}

#endif //!SRC_DS_APP_ENGINE_RENDERERS_ENGINE_CONTINUOUS_RENDERER_H_