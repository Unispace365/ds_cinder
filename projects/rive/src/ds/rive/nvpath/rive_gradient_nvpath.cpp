#include "stdafx.h"

#include "ds/rive/nvpath/rive_gradient_nvpath.h"

namespace ds {

rive::rcp<RiveGradientNvPath> RiveGradientNvPath::makeLinearGradient(float sx, float sy, float ex, float ey,
														 const rive::ColorInt colors[], const float stops[],
														 size_t count) {
	auto result = rive::rcp(new RiveGradientNvPath());

	const auto uid = pointerToUid(result.get());
	result->mPaint = nvpath::Paint::linear(uid);
	result->mPaint.setCoords0(sx, sy);
	result->mPaint.setCoords1(ex, ey);
	if (count) {
		result->mPaint.clear();
		for (size_t i = 0; i < count; ++i)
			result->mPaint.set(stops[i], ci::ColorA8u::hexA(colors[i]));
	}
	result->mPaint.setUseObjectBoundingBox(false);

	return result;
}

rive::rcp<RiveGradientNvPath> RiveGradientNvPath::makeRadialGradient(float cx, float cy, float radius,
														 const rive::ColorInt colors[], const float stops[],
														 size_t count) {
	auto result = rive::rcp(new RiveGradientNvPath());

	const auto uid = pointerToUid(result.get());
	result->mPaint = nvpath::Paint::radial(uid);
	result->mPaint.setCoords0(cx, cy);
	result->mPaint.setRadius0(radius);
	if (count) {
		result->mPaint.clear();
		for (size_t i = 0; i < count; ++i)
			result->mPaint.set(stops[i], ci::ColorA8u::hexA(colors[i]));
	}
	result->mPaint.setUseObjectBoundingBox(false);

	return result;
}

} // namespace ds