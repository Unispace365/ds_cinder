#include "ds/arc/arc_chain.h"

#include "ds/arc/arc_io.h"

namespace ds {
namespace arc {

/**
 * ds::arc::Chain
 */
Chain::Chain(){
}

double Chain::run(const Input& input, const double v) const {
	double		ans = v;
	for (auto it=mArc.begin(), end=mArc.end(); it!=end; ++it) {
		const Arc*		a = it->get();
		if (a) ans = a->run(input, ans);
	}
	return ans;
}

void Chain::renderCircle(const Input& input, RenderCircleParams& p) const {
	for (auto it=mArc.begin(), end=mArc.end(); it!=end; ++it) {
		const Arc*		a = it->get();
		if (a) a->renderCircle(input, p);
	}
}

void Chain::readXml(const ci::XmlTree& xml) {
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
