#include "ds/arc/arc_pow.h"

namespace ds {
namespace arc {

/**
 * ds::arc::Pow
 */
Pow::Pow()
	: mExp(1.0)
{
}

double Pow::run(const Input& input, const double v) const
{
	return pow(v, mExp.getValue(input));
}

void Pow::readXml(const ci::XmlTree& xml)
{
	mExp = FloatParam(1.0);

	for (auto it=xml.begin(), end=xml.end(); it != end; ++it) {
		if (it->getTag() == "exp") {
			mExp.readXml(*it);
			mExp.mValue = it->getAttributeValue<double>("value", mExp.mValue);
		}
	}
}

} // namespace arc
} // namespace ds
