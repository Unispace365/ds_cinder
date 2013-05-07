#include "ds/arc/arc_color_array.h"

#include <boost/tokenizer.hpp>
#include <Poco/String.h>
#include "ds/util/string_util.h"

namespace ds {
namespace arc {

/**
 * ds::arc::ColorArray
 */
ColorArray::ColorArray()
{
}

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

void ColorArray::readXml(const ci::XmlTree& xml)
{
	mColor.clear();

	for (auto it=xml.begin(), end=xml.end(); it != end; ++it) {
		if (it->getTag() == "color") {
			ci::ColorA					clr;
			const std::string		rgb = it->getAttributeValue<std::string>("rgb", "");
			if (parse_color(rgb, clr)) {
				mColor.push_back(clr);
			}
		}
	}
}

} // namespace arc
} // namespace ds
