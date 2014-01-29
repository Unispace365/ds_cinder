#include "ds/arc/arc_map.h"

namespace ds {
namespace arc {

namespace {

void		read_float_param(const ci::XmlTree& xml, FloatParam& fp) {
	fp.readXml(xml);
	fp.mValue = xml.getAttributeValue<double>("value", fp.mValue);
}

}

/**
 * ds::arc::Map
 */
Map::Map()
		: mFromMin(0.0)
		, mFromMax(1.0)
		, mToMin(0.0)
		, mToMax(1.0) {
}

double Map::run(const Input& input, const double v) const {
	double			ans = v;
	const double	from_min = mFromMin.getValue(input),
					from_max = mFromMax.getValue(input),
					to_min = mToMin.getValue(input),
					to_max = mToMax.getValue(input);
	// Clip to from range
	if (ans < from_min) ans = from_min;
	else if (ans > from_max) ans = from_max;
	// Convert to a unit value in the from range
	ans = (ans-from_min) / (from_max-from_min);
	// Convert to to range
	ans = to_min + (ans * (to_max-to_min));
	return ans;
}

void Map::readXml(const ci::XmlTree& xml) {
	mFromMin = FloatParam(0.0);
	mFromMax = FloatParam(1.0);
	mToMin = FloatParam(0.0);
	mToMax = FloatParam(1.0);

	for (auto it=xml.begin(), end=xml.end(); it != end; ++it) {
		if (it->getTag() == "from_min") read_float_param(*it, mFromMin);
		else if (it->getTag() == "from_max") read_float_param(*it, mFromMax);
		else if (it->getTag() == "to_min") read_float_param(*it, mToMin);
		else if (it->getTag() == "to_max") read_float_param(*it, mToMax);
	}
}

} // namespace arc
} // namespace ds
