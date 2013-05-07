#include "ds/arc/arc_layer.h"

#include "ds/arc/arc_io.h"

#include "ds/arc/arc_render_circle.h"

namespace ds {
namespace arc {

/**
 * ds::arc::Layer
 */
Layer::Layer()
	: mInput([](const RenderCircleParams& p)->double{return p.mDist;})
{
}

void Layer::renderCircle(RenderCircleParams& p) const
{
	p.mOutput = mColor.at(mInput(p));
}

void Layer::readXml(const ci::XmlTree& xml)
{
	mArc.reset();

	{
		const std::string		inp = xml.getAttributeValue<std::string>("input", "");
		if (inp == "degree") mInput = [](const RenderCircleParams& p)->double{return p.mDegree;};
		else mInput = [](const RenderCircleParams& p)->double{return p.mDist;};
	}

	for (auto it=xml.begin(), end=xml.end(); it != end; ++it) {
		std::unique_ptr<Arc>		a(ds::arc::create(*it));
		if (a) {
			mArc = std::move(a);
			return;
		}
	}

	mColor.readXml(xml);
}

} // namespace arc
} // namespace ds
