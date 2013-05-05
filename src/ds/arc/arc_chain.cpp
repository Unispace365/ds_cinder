#include "ds/arc/arc_chain.h"

#include "ds/arc/arc_io.h"

namespace ds {
namespace arc {

/**
 * ds::arc::Chain
 */
Chain::Chain()
{
}

void Chain::readXml(const ci::XmlTree& xml)
{
	mArc.clear();

	for (auto it=xml.begin(), end=xml.end(); it != end; ++it) {
		std::unique_ptr<Arc>		a(ds::arc::create(*it));
		if (a) {
			mArc.push_back(std::move(a));
		}
	}
}

} // namespace arc
} // namespace ds
