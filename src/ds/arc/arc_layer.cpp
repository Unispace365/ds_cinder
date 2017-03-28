#include "stdafx.h"

#include "ds/arc/arc_layer.h"

#include <Poco/String.h>
#include "ds/arc/arc_io.h"
#include "ds/arc/arc_render_circle.h"
#include "ds/math/math_func.h"

namespace ds {
namespace arc {

/**
 * ds::arc::Layer
 */
Layer::Layer()
	: mScaleMode(SCALE_MULTIPLY)
	, mScale(1.0)
{
	setInput(INPUT_DIST);
	setScale(SCALE_MULTIPLY, 1.0);
	setCompositeMode(COMPOSITE_SRCOVER);
}

void Layer::renderCircle(const Input& ip, RenderCircleParams& p) const
{
	// Determine my input based on my parameters
	const double	x = p.mX,
					y = p.mY;
	if (x < 0 || y < 0 || x >= p.mW || y >= p.mH) return;
	ci::dvec2		offset = mOffset.getValue(ip);
	const double	cenx = p.mCenX + offset.x,
					ceny = p.mCenY + offset.y;
	const double	dist = ds::math::dist(cenx, ceny, x, y);
	const double	max_dist = mScaleFn(p, offset);
	if (dist >= max_dist) return;

	const double	unit_dist = 1.0 - ds::math::clamp(dist / max_dist, 0.0, 1.0);
	const double	unit_degree = ds::math::clamp(ds::math::degree(x - cenx, ceny - y) / 360.0, 0.0, 1.0);

	// render
	double			input = mInputFn(unit_dist, unit_degree);
	if (mArc) {
		input = mArc->run(ip, input);
	}
	ci::ColorA		clr = mColor.at(ip, input);

	// draw into params
	if (clr.a > 0.0f) {
		// antialias
		if (dist > (max_dist - 1.0)) {
			clr.a *= static_cast<float>(max_dist-dist);
		}
		mCompositeFn(p.mOutput, clr.premultiplied());
	}
}

void Layer::readXml(const ci::XmlTree& xml)
{
	mArc.reset();

	// Input mode
	{
		const std::string		inp = xml.getAttributeValue<std::string>("input", "");
		if (inp == "degree") setInput(INPUT_DEGREE);
		else setInput(INPUT_DIST);
	}

	for (auto it=xml.begin(), end=xml.end(); it != end; ++it) {
		if (it->getTag() == "offset") {
			mOffset.readXml(*it);
			mOffset.mValue.x = it->getAttributeValue<double>("x", 0.0);
			mOffset.mValue.y = it->getAttributeValue<double>("y", 0.0);
		} else if (it->getTag() == "scale") {
			setScale(	it->getAttributeValue<std::string>("mode", ""),
						it->getAttributeValue<double>("amount", 1.0));
		} else if (it->getTag() == "color") {
			// Colors handled by the color reader below
		} else if (it->getTag() == "arc") {
			std::unique_ptr<Arc>		a(ds::arc::create(*it));
			if (a) {
				mArc = std::move(a);
			}
		}
	}

	mColor.readXml(xml);
}

void Layer::setScale(const ScaleMode& mode, const double amount)
{
	mScale = amount;
	if (mode == SCALE_FIT) {
		mScaleMode = SCALE_FIT;
		// The fit mode acounts for my current offset.
		mScaleFn = [this](const RenderCircleParams& p, const ci::dvec2& offset)->double{
			const double	cenx = p.mCenX - (abs(offset.x)),
							ceny = p.mCenY - (abs(offset.y));
			const double	max_dist = ds::math::dist(cenx, ceny, cenx, 0.0);
			return max_dist * this->mScale;
		};
	} else {
		mScaleMode = SCALE_MULTIPLY;
		mScaleFn = [this](const RenderCircleParams& p, const ci::dvec2&)->double{return p.mMaxDist * this->mScale;};
	}
}

void Layer::setScale(const std::string& in_mode, const double amount)
{
	const std::string	mode = Poco::toLower(in_mode);
	if (mode.find("fit") != std::string::npos) setScale(SCALE_FIT, amount);
	else setScale(SCALE_MULTIPLY, amount);
}

void Layer::setInput(const InputMode mode)
{
	if (mode == INPUT_DEGREE) mInputFn = [](const double dist, const double degree)->double{return degree;};
	else mInputFn = [](const double dist, const double degree)->double{return dist;};
}

void Layer::setCompositeMode(const CompositeMode mode)
{
	mCompositeFn = [](ci::ColorA& dst, const ci::ColorA& src) {
		const float		amt = (1-src.a);
		dst.r = src.r + (amt * dst.r);
		dst.g = src.g + (amt * dst.g);
		dst.b = src.b + (amt * dst.b);
		dst.a = src.a + (amt * dst.a);
	};
}

} // namespace arc
} // namespace ds
