#include "stdafx.h"

#include "rive/rive_gradient_nvpath.h"

#include <rive/shapes/shape_paint_path.hpp>

namespace ds { namespace ui {

	rive::rcp<RiveGradient> RiveGradient::makeLinearGradient(float sx, float sy, float ex, float ey,
															 const rive::ColorInt colors[], const float stops[],
															 size_t count) {
		auto result = rive::rcp(new RiveGradient());

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

	rive::rcp<RiveGradient> RiveGradient::makeRadialGradient(float cx, float cy, float radius,
															 const rive::ColorInt colors[], const float stops[],
															 size_t count) {
		auto result = rive::rcp(new RiveGradient());

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

}} // namespace ds::ui