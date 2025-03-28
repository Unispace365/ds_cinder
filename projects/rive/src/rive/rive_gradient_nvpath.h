#pragma once

#include <iomanip>
#include <sstream>
#include <string>

#include <rive/refcnt.hpp>
#include <rive/renderer.hpp>

#include <nvpath/nv_path.h>

namespace ds { namespace ui {

	class RiveGradient : public rive::RenderShader {
	  public:
		RiveGradient() = default;

		const nvpath::Paint& getPaint() const { return mPaint; }

		static std::string pointerToUid(const void* ptr) {
			std::ostringstream oss;
			oss << std::hex << std::setw(sizeof(ptr) * 2) << std::setfill('0') << reinterpret_cast<uintptr_t>(ptr);
			return oss.str();
		}

		static rive::rcp<RiveGradient> makeLinearGradient(float sx, float sy, float ex, float ey,
														  const rive::ColorInt colors[], const float stops[],
														  size_t count);

		static rive::rcp<RiveGradient> makeRadialGradient(float cx, float cy, float radius,
														  const rive::ColorInt colors[], const float stops[],
														  size_t count);

	  private:
		nvpath::Paint mPaint;
	};

}} // namespace ds::ui