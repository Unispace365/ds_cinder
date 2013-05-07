#include "ds/arc/arc_layer.h"

#include "ds/arc/arc_io.h"

namespace ds {
namespace arc {

/**
 * ds::arc::Layer
 */
Layer::Layer()
{
}

void Layer::renderCircle(RenderCircleParams& p) const
{
}

void Layer::readXml(const ci::XmlTree& xml)
{
	mArc.reset();

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
