#pragma once

#include <rive/factory.hpp>
#include <rive/refcnt.hpp>

namespace ds { namespace ui {

	class RiveFactoryNvPath : public rive::Factory {
	  public:
		rive::rcp<rive::RenderBuffer> makeRenderBuffer(rive::RenderBufferType, rive::RenderBufferFlags,
													   size_t sizeInBytes) override;

		rive::rcp<rive::RenderShader> makeLinearGradient(float sx, float sy, float ex, float ey,
														 const rive::ColorInt colors[], // [count]
														 const float		  stops[],	// [count]
														 size_t				  count) override;

		rive::rcp<rive::RenderShader> makeRadialGradient(float cx, float cy, float radius,
														 const rive::ColorInt colors[], // [count]
														 const float		  stops[],	// [count]
														 size_t				  count) override;


		rive::rcp<rive::RenderPath> makeRenderPath(rive::RawPath&, rive::FillRule) override;

		// Deprecated -- working to make RenderPath's immutable
		rive::rcp<rive::RenderPath> makeEmptyRenderPath() override;

		rive::rcp<rive::RenderPaint> makeRenderPaint() override;

		rive::rcp<rive::RenderImage> decodeImage(rive::Span<const uint8_t>) override { return {}; }
	};

}} // namespace ds::ui