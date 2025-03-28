#pragma once

#include <rive/refcnt.hpp>
#include <rive/renderer.hpp>

#include <nvpath/nv_path.h>

#include "ds/rive/nvpath/rive_path_nvpath.h"

namespace ds {

class RivePaintNvPath : public rive::RenderPaint {
  public:
	RivePaintNvPath() = default;

	void style(rive::RenderPaintStyle style) override;
	void color(rive::ColorInt value) override;
	void thickness(float value) override;
	void join(rive::StrokeJoin value) override;
	void cap(rive::StrokeCap value) override;
	void feather(float value) override {} // Not supported.
	void blendMode(rive::BlendMode value) override;
	void shader(rive::rcp<rive::RenderShader>) override;
	void invalidateStroke() override {}

	void render(const RivePathNvPath* path) const;

  private:
	nvpath::Paint	  mPaint;
	nvpath::CapsStyle mCapsStyle;
	nvpath::JoinStyle mJoinStyle;
	float			  mStrokeWidth{1.0f};
	bool			  mIsStroke{false};
};

} // namespace ds