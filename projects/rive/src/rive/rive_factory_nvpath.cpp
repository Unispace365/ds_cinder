#include "stdafx.h"

#include "rive/rive_factory_nvpath.h"
#include "rive/rive_gradient_nvpath.h"
#include "rive/rive_paint_nvpath.h"

#include <rive/shapes/shape_paint_path.hpp>

namespace ds { namespace ui {

	rive::rcp<rive::RenderBuffer> RiveFactoryNvPath::makeRenderBuffer(rive::RenderBufferType, rive::RenderBufferFlags,
																	  size_t sizeInBytes) {
		return {}; // We don't need a render buffer.
	}

	rive::rcp<rive::RenderShader> RiveFactoryNvPath::makeLinearGradient(float sx, float sy, float ex, float ey,
																		const rive::ColorInt colors[],
																		const float stops[], size_t count) {
		return RiveGradient::makeLinearGradient(sx, sy, ex, ey, colors, stops, count);
	}

	rive::rcp<rive::RenderShader> RiveFactoryNvPath::makeRadialGradient(float cx, float cy, float radius,
																		const rive::ColorInt colors[],
																		const float stops[], size_t count) {
		return RiveGradient::makeRadialGradient(cx, cy, radius, colors, stops, count);
	}

	rive::rcp<rive::RenderPath> RiveFactoryNvPath::makeRenderPath(rive::RawPath& rawPath, rive::FillRule fillRule) {
		return rive::rcp(new RivePath(rawPath, fillRule));
	}

	rive::rcp<rive::RenderPath> RiveFactoryNvPath::makeEmptyRenderPath() {
		return rive::rcp(new RivePath());
	}

	rive::rcp<rive::RenderPaint> RiveFactoryNvPath::makeRenderPaint() {
		return rive::rcp(new RivePaint());
	}

}} // namespace ds::ui