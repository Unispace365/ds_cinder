#include "stdafx.h"

#include "rive/rive_paint_nvpath.h"
#include "rive/rive_gradient_nvpath.h"

namespace ds { namespace ui {

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

}} // namespace ds::ui