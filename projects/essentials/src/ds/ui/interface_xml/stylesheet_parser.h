#pragma once
#ifndef DS_UI_STYLESHEET_PARSER_H_
#define DS_UI_STYLESHEET_PARSER_H_

#include <string>
#include <vector>

#include <boost/variant.hpp>

namespace ds {
namespace ui {

struct Stylesheet;

namespace stylesheets {

struct IdSelector {
	std::string selector;
};

struct ClassSelector {
	std::string selector;
};

typedef boost::variant<IdSelector, ClassSelector> Selector;

struct Property {
	std::string property_name;
	std::string property_value;
};

struct Rule {
	std::vector< std::vector< Selector > > matchers;
	std::vector<Property> properties;
};

typedef	std::vector< Rule > Rules;

template< typename Iter >
bool parse_stylesheet(Iter first, Iter last, Rules& s);

} // namespace stylesheets

struct Stylesheet {
	stylesheets::Rules mRules;
	std::string mReferer;
	bool loadFile( const std::string &filename, const std::string &referer );
};

}} // namespace ds::ui
#endif // DS_UI_STYLESHEET_PARSER_H_
