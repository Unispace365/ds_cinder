#include "stdafx.h"

#include "rive/rive_path_nvpath.h"

#include <rive/shapes/shape_paint_path.hpp>

namespace ds { namespace ui {

	RivePath::RivePath(const rive::RawPath& path, rive::FillRule fillRule) {}

	void RivePath::addRenderPath(RenderPath* path, const rive::Mat2D& transform) {
		auto shape = dynamic_cast<rive::ShapePaintPath*>(path);
		if (shape) {}
	}

	void RivePath::addRawPath(const rive::RawPath& path) {
		constexpr std::array<GLubyte, 6> mapping = {GL_MOVE_TO_NV, GL_LINE_TO_NV,		 GL_QUADRATIC_CURVE_TO_NV,
													0 /* CONIC */, GL_CUBIC_CURVE_TO_NV, GL_CLOSE_PATH_NV};

		std::vector<GLubyte> commands;
		commands.reserve(path.verbs().size());

		for (auto command : path.verbs()) {
			commands.push_back(mapping[int(command)]);
		}

		nvpath::PathHelper helper;
		helper.setCoords(reinterpret_cast<const float*>(path.points().data()), path.points().size() * 2);
		helper.setCommands(commands);

		mPath += nvpath::Path(helper);
	}

	void RivePath::rewind() {
		mPath = nvpath::Path();
	}

	void RivePath::fillRule(rive::FillRule value) {
		/* not supported yet */
	}

	void RivePath::addPath(CommandPath* path, const rive::Mat2D& transform) {
		RenderPath::addPath(path, transform);
	}

	void RivePath::moveTo(float x, float y) {
		mHelper.moveTo(x, y);
	}

	void RivePath::lineTo(float x, float y) {
		mHelper.lineTo(x, y);
	}

	void RivePath::cubicTo(float ox, float oy, float ix, float iy, float x, float y) {
		mHelper.curveTo(ox, oy, ix, iy, x, y);
	}

	void RivePath::close() {
		mHelper.close();
	}

	rive::RenderPath* RivePath::renderPath() {
		mPath = nvpath::Path(mHelper);
		return this;
	}

	const rive::RenderPath* RivePath::renderPath() const {
		mPath = nvpath::Path(mHelper);
		return this;
	}

}} // namespace ds::ui