#include "stdafx.h"

#include "rive/RiveRendererNvPath.h"

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

	void RivePaint::style(rive::RenderPaintStyle style) {
		mIsStroke = style == rive::RenderPaintStyle::stroke;
	}

	void RivePaint::color(rive::ColorInt value) {
		mPaint = nvpath::Paint(ci::ColorA8u::hexA(value));
	}

	void RivePaint::thickness(float value) {
		mStrokeWidth = value;
	}

	void RivePaint::join(rive::StrokeJoin value) {
		switch (value) {
		case rive::StrokeJoin::miter:
			mJoinStyle = nvpath::JoinStyle::MITER_REVERT;
			break;
		case rive::StrokeJoin::round:
			mJoinStyle = nvpath::JoinStyle::ROUND;
			break;
		case rive::StrokeJoin::bevel:
			mJoinStyle = nvpath::JoinStyle::BEVEL;
			break;
		default:
			mJoinStyle = nvpath::JoinStyle::DEFAULT;
		}
	}

	void RivePaint::cap(rive::StrokeCap value) {
		switch (value) {
		case rive::StrokeCap::butt:
			mCapsStyle = nvpath::CapsStyle::FLAT;
			break;
		case rive::StrokeCap::round:
			mCapsStyle = nvpath::CapsStyle::ROUND;
			break;
		case rive::StrokeCap::square:
			mCapsStyle = nvpath::CapsStyle::SQUARE;
			break;
		default:
			mCapsStyle = nvpath::CapsStyle::DEFAULT;
		}
	}

	void RivePaint::blendMode(rive::BlendMode value) {
		/* not supported yet */
	}

	void RivePaint::shader(rive::rcp<rive::RenderShader> shader) {
		auto theShader = dynamic_cast<RiveGradient*>(shader.get());
		if (theShader) mPaint = theShader->getPaint();
	}

	void RivePaint::render(const RivePath* path) const {
		if (mIsStroke) {
			path->getPath().setJoinStyle(mJoinStyle);
			path->getPath().setEndCaps(mCapsStyle);
			path->getPath().setMiterLimit(10.0f);
			path->getPath().setStrokeWidth(mStrokeWidth);
			path->getPath().stroke(mPaint);
		} else {
			path->getPath().fill(mPaint);
		}
	}

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

	void RiveRendererNvPath::save() {
		mTransformStack.push_back(mTransformStack.back());
		mClipCountStack.push_back(mClipCountStack.back());
	}

	void RiveRendererNvPath::restore() {
		mTransformStack.pop_back();

		size_t clipCount = mClipCountStack.back();
		mClipCountStack.pop_back();
		while (clipCount-- > mClipCountStack.back())
			nvpath::Path::popClipPath();
	}

	void RiveRendererNvPath::transform(const rive::Mat2D& transform) {
		mTransformStack.back() =
			mTransformStack.back() * glm::mat4(transform[0], transform[1], 0, 0, transform[2], transform[3], 0, 0, 0, 0,
											   1, 0, transform[4], transform[5], 0, 1);
	}

	void RiveRendererNvPath::drawPath(rive::RenderPath* path, rive::RenderPaint* paint) {
		auto thePath  = dynamic_cast<RivePath*>(path);
		auto thePaint = dynamic_cast<RivePaint*>(paint);
		if (thePath && thePaint) {
			ci::gl::ScopedModelMatrix sm;
			ci::gl::multModelMatrix(mTransformStack.back());

			thePaint->render(thePath);
		}
	}

	void RiveRendererNvPath::clipPath(rive::RenderPath* path) {
		auto thePath = dynamic_cast<RivePath*>(path);
		if (thePath) {
			nvpath::Path::pushClipPath(thePath->getPath());
			++mClipCountStack.back();
		}
	}

	void RiveRendererNvPath::drawImage(const rive::RenderImage*, rive::BlendMode, float opacity) {}

	void RiveRendererNvPath::drawImageMesh(const rive::RenderImage*, rive::rcp<rive::RenderBuffer> vertices_f32,
										   rive::rcp<rive::RenderBuffer> uvCoords_f32,
										   rive::rcp<rive::RenderBuffer> indices_u16, uint32_t vertexCount,
										   uint32_t indexCount, rive::BlendMode, float opacity) {}

}} // namespace ds::ui