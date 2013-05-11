#pragma once
#ifndef DS_ARC_ARCLAYER_H_
#define DS_ARC_ARCLAYER_H_

#include <memory>
#include "ds/arc/arc.h"
#include "ds/arc/arc_color_array.h"

namespace ds {
namespace arc {

/**
 * \class ds::arc::Layer
 * \brief A special arc used to render layers of an image.
 */
class Layer : public Arc
{
public:
	Layer();

	virtual void			renderCircle(const Input&, RenderCircleParams&) const;

	virtual void			readXml(const ci::XmlTree&);

private:
	enum ScaleMode { SCALE_MULTIPLY, SCALE_FIT };
	enum InputMode { INPUT_DIST, INPUT_DEGREE };
	enum CompositeMode { COMPOSITE_SRCOVER };

	void					setScale(const ScaleMode& mode, const double value);
	void					setScale(const std::string& mode, const double value);
	void					setInput(const InputMode);
	void					setCompositeMode(const CompositeMode);

	ci::Vec2d				mOffset;
	ScaleMode				mScaleMode;
	double					mScale;
	std::unique_ptr<Arc>	mArc;
	ColorArray				mColor;
	std::function<double(const RenderCircleParams&)>
							mScaleFn;
	std::function<double(const double dist, const double degree)>
							mInputFn;
	std::function<void(ci::ColorA& dst, const ci::ColorA& src)>
							mCompositeFn;
};

} // namespace arc
} // namespace ds

#endif // DS_ARC_ARCRENDER_H_