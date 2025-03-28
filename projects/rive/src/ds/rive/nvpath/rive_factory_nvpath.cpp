#include "stdafx.h"

#include "ds/rive/nvpath/rive_factory_nvpath.h"
#include "ds/rive/nvpath/rive_gradient_nvpath.h"
#include "ds/rive/nvpath/rive_paint_nvpath.h"

namespace ds {

rive::rcp<rive::RenderBuffer> RiveFactoryNvPath::makeRenderBuffer(rive::RenderBufferType, rive::RenderBufferFlags,
																  size_t sizeInBytes) {
	return {}; // We don't need a render buffer.
}

rive::rcp<rive::RenderShader> RiveFactoryNvPath::makeLinearGradient(float sx, float sy, float ex, float ey,
																	const rive::ColorInt colors[], const float stops[],
																	size_t count) {
	return RiveGradientNvPath::makeLinearGradient(sx, sy, ex, ey, colors, stops, count);
}

rive::rcp<rive::RenderShader> RiveFactoryNvPath::makeRadialGradient(float cx, float cy, float radius,
																	const rive::ColorInt colors[], const float stops[],
																	size_t count) {
	return RiveGradientNvPath::makeRadialGradient(cx, cy, radius, colors, stops, count);
}

rive::rcp<rive::RenderPath> RiveFactoryNvPath::makeRenderPath(rive::RawPath& rawPath, rive::FillRule fillRule) {
	return rive::rcp(new RivePathNvPath(rawPath, fillRule));
}

rive::rcp<rive::RenderPath> RiveFactoryNvPath::makeEmptyRenderPath() {
	return rive::rcp(new RivePathNvPath());
}

rive::rcp<rive::RenderPaint> RiveFactoryNvPath::makeRenderPaint() {
	return rive::rcp(new RivePaintNvPath());
}

} // namespace ds