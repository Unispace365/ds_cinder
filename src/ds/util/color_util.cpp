#include "color_util.h"

#include <sstream>

namespace ds {

namespace {

int				parse_component(const std::string& input, const size_t pos, const int defaultValue = 0) {
	if (input.size() <= pos+1) return defaultValue;

	unsigned int x;
	const std::string	hex(input.substr(pos, 2));
    std::stringstream ss;
    ss << std::hex << hex;
	ss >> x;
	int		ans = static_cast<int>(x);
	if (ans <= 0) return 0;
	if (ans >= 255) return 255;
	return ans;
}

}

/**
 * \function ds::parse_color
 */
ci::ColorA parse_color(const std::string& input) {
	if (input.size() < 3) throw std::runtime_error("parse_color() invalid input (" + input + ")");
	// Hex
	if (input[0] == '#' || input[0] == 'x') {
		const int	r = parse_component(input, 1, 0);
		const int	g = parse_component(input, 3, 0);
		const int	b = parse_component(input, 5, 0);
		const int	a = parse_component(input, 7, 255);
		return ci::ColorA(r/255.0f, g/255.0f, b/255.0f, a/255.0f);
	// Common input formats
	} else if (input.size() == 6) {
		const int	r = parse_component(input, 0, 0);
		const int	g = parse_component(input, 2, 0);
		const int	b = parse_component(input, 4, 0);
		return ci::ColorA(r/255.0f, g/255.0f, b/255.0f, 1.0f);
	}
	throw std::runtime_error("parse_color() invalid input (" + input + ")");
}

ci::ColorA parse_color(const std::string& input, const ci::ColorA& dv) {
	try {
		return parse_color(input);
	} catch (std::exception const&) {
	}
	return dv;
}

} // namespace ds