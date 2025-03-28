#pragma once

#include <rive/renderer.hpp>

#include <nvpath/nv_path.h>

namespace ds {

class RivePathNvPath : public rive::RenderPath {
  public:
	RivePathNvPath() = default;
	RivePathNvPath(const rive::RawPath& path, rive::FillRule fillRule);

	void addRenderPath(RenderPath* path, const rive::Mat2D& transform) override;
	void addRawPath(const rive::RawPath& path) override;

	void rewind() override;
	void fillRule(rive::FillRule value) override;
	void addPath(CommandPath* path, const rive::Mat2D& transform) override;

	void moveTo(float x, float y) override;
	void lineTo(float x, float y) override;
	void cubicTo(float ox, float oy, float ix, float iy, float x, float y) override;
	void close() override;

	RenderPath*		  renderPath() override;
	const RenderPath* renderPath() const override;

	nvpath::Path& getPath() const { return mPath; }

  private:
	mutable nvpath::Path mPath;
	nvpath::PathHelper	 mHelper;
};

} // namespace ds