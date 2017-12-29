#include "stdafx.h"

#include "ds/arc/arc_color_array.h"

#include <boost/tokenizer.hpp>
#include <Poco/String.h>
#include <cinder/Xml.h>
#include "ds/util/string_util.h"

namespace ds {
namespace arc {

static bool parse_color(const std::string& str, ci::ColorA& clr);

/**
 * ds::arc::ColorArray
 */
ColorArray::ColorArray()
{
}


ci::ColorA ColorArray::at(const Input& input, const double unit) const
{
	if (mColor.empty()) {
		return ci::ColorA(0.0, 0.0, 0.0, static_cast<float>(unit));
	}
	if (mColor.size() == 1) {
		ci::ColorA		c = mColor[0].getValue(input);
		return ci::ColorA(c.r, c.g, c.b, c.a * static_cast<float>(unit));
	}

return ci::ColorA(0.0, 0.0, 0.0, static_cast<float>(unit));
}

void ColorArray::readXml(const ci::XmlTree& xml)
{
	mColor.clear();

	for (auto it=xml.begin(), end=xml.end(); it != end; ++it) {
		if (it->getTag() == "color") {
			ci::ColorA				clr;
			const std::string		rgb = it->getAttributeValue<std::string>("rgb", "");
			if (parse_color(rgb, clr)) {
				ColorParam			cp(clr);
				cp.readXml(*it);
				mColor.push_back(cp);
			}
		}
	}
}

/**
 * Misc
 */
static float constrain(float v)
{
	v /= 255.0f;
	if (v <= 0.0f) return 0.0f;
	if (v >= 1.0f) return 1.0f;
	return v;
}

static bool parse_color(const std::string& str, ci::ColorA& clr)
{
	if (str.empty()) return false;

	boost::char_separator<char>	sep(",");
	boost::tokenizer<boost::char_separator<char>> tokens(str, sep);
	float			v[4];
	v[0] = 0.0f;	v[1] = 0.0f;	v[2] = 0.0f;	v[3] = 255.0f;
	int				idx = 0;
	for (auto it=tokens.begin(); it != tokens.end(); ++it) {
		ds::string_to_value(Poco::trim(*it), v[idx++]);
		if (idx >= 4) break;
	}
	clr.r = constrain(v[0]);	clr.g = constrain(v[1]);	clr.b = constrain(v[2]);	clr.a = constrain(v[3]);
	return true;
}

} // namespace arc
} // namespace ds
