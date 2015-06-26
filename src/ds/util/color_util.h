#pragma once
#ifndef DS_UTIL_COLORUTIL_H_
#define DS_UTIL_COLORUTIL_H_

#include <cinder/Color.h>

namespace ds {

	/** Read a colour from a string of the format \code #rrggbb, xrrggbb or rrggbb. For example: #ffcc00. \endcode Throws exception on error on error. */
	ci::Color  parse_color(const std::string& input);

	/** Read a colour from a string of the format \code #rrggbb, xrrggbb or rrggbb. For example: #ffcc00. \endcode Returns the defaultColor if the string can't be found.*/
	ci::Color  parse_color(const std::string& input, const ci::Color& defaultColor);

	/** Read a colour from a string of the format \code #rrggbbaa, xrrggbbaa or rrggbbaa. For example: #ffcc0033. \endcode Throws exception on error on error. */
	ci::ColorA parse_colora(const std::string& input);

	/** Read a colour from a string of the format  \code #rrggbbaa, xrrggbbaa or rrggbbaa. For example: #ffcc0033. \endcode  Returns the defaultColor if the string can't be found. */
	ci::ColorA parse_colora(const std::string& input, const ci::ColorA& defaultColor);

} // namespace ds

#endif // DS_UTIL_COLORUTIL_H_
