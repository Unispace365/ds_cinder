#pragma once
#ifndef DS_UTIL_COLORUTIL_H_
#define DS_UTIL_COLORUTIL_H_

#include <cinder/Color.h>

namespace ds {

/**
 * \function ds::parse_color
 * \brief Read a colour from a string. The version with no
 * default throws on error.
 */
ci::Color  parse_color(const std::string& input);
ci::Color  parse_color(const std::string& input, const ci::Color&);
ci::ColorA parse_colora(const std::string& input);
ci::ColorA parse_colora(const std::string& input, const ci::ColorA&);

} // namespace ds

#endif // DS_UTIL_COLORUTIL_H_
