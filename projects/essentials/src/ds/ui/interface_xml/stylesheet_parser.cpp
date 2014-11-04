#include "stylesheet_parser.h"

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/foreach.hpp>

#include <ds/debug/logger.h>

#include <fstream>

BOOST_FUSION_ADAPT_STRUCT(
	ds::ui::stylesheets::IdSelector,
	(std::string, selector)
)

BOOST_FUSION_ADAPT_STRUCT(
	ds::ui::stylesheets::ClassSelector,
	(std::string, selector)
)

BOOST_FUSION_ADAPT_STRUCT(
	ds::ui::stylesheets::Property,
	(std::string, property_name)
	(std::string, property_value)
)

BOOST_FUSION_ADAPT_STRUCT(
	ds::ui::stylesheets::Rule,
	(std::vector< std::vector< ds::ui::stylesheets::Selector > >, matchers)
	(std::vector<ds::ui::stylesheets::Property>, properties)
)

BOOST_FUSION_ADAPT_STRUCT(
	ds::ui::Stylesheet,
	(std::vector<ds::ui::stylesheets::Rule>, mRules)
)

namespace ds {
namespace ui {
namespace stylesheets {

namespace spirit = boost::spirit;
namespace fusion = boost::fusion;
namespace phoenix = boost::phoenix;
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

// ----------------------------------------------------------------------
using ascii::char_;
using qi::lit;

// Comment and whites-space skipper
template<typename Iterator>
struct custom_skipper : public qi::grammar<Iterator> {
	custom_skipper() : custom_skipper::base_type(skip) {
		skip = ascii::space | (lit("/*") >> *(!lit("*/") >> char_) >> lit("*/"));
	}
	qi::rule<Iterator> skip;
};

// The stylesheet grammar
template <typename Iterator, typename Skipper=custom_skipper<Iterator> >
struct stylesheet_grammar : qi::grammar<Iterator, Rules(), Skipper> {
	stylesheet_grammar()
		: stylesheet_grammar::base_type(rules)
	{
		using qi::lexeme;
		using namespace qi::labels;

		nmstart				%= char_("a-zA-Z") | char_('_');
		nmchar				%= nmstart | char_("0-9") | char_('-');
		identifier			%= nmstart >> *(nmchar);

		property_name		%= identifier;
		property_value		%= lexeme[ +(char_ - '{' - '}' - ';') ];
		prop				%=  property_name >>
								':' >>
								property_value
								;

		id_selector			%= ( lit('#') >> identifier );
		class_selector		%= ( lit('.') >> identifier );
		selector			%= ( id_selector | class_selector );

		selectors			%= ( + ( selector ) );
		rule				%= (selectors % ',' ) >> lit('{') >> (prop % ';') >> -(lit(';')) >> lit('}');
		rules				%= *(rule);
	}

	qi::rule<Iterator, char(), Skipper>				nmstart;
	qi::rule<Iterator, char(), Skipper>				nmchar;
	qi::rule<Iterator, std::string(), Skipper>		identifier;

	qi::rule<Iterator, IdSelector(), Skipper>		id_selector;
	qi::rule<Iterator, ClassSelector(), Skipper>	class_selector;
	qi::rule<Iterator, Selector(), Skipper>			selector;
	qi::rule<Iterator, std::vector<Selector>(), Skipper>
													selectors;

	qi::rule<Iterator, std::string(), Skipper> property_name;
	qi::rule<Iterator, std::string(), Skipper> property_value;
	qi::rule<Iterator, Property(), Skipper> prop;

	qi::rule<Iterator, Rule(), Skipper> rule;
	qi::rule<Iterator, Rules(), Skipper> rules;
};

template< typename Iter >
bool parse_stylesheet(Iter first, Iter last, Rules& s) {
	/*
	 * selector [, selector]* {
	 *	   property: value(; property: value)*(;?)
	 * }
	 */

	stylesheet_grammar<Iter> grammar;
	custom_skipper<Iter> skipper;
	bool r = qi::phrase_parse( first, last, grammar, skipper, s );

	if (r && first == last) {
		return true;
	}
	else {
		std::string context(first, last);
		DS_LOG_WARNING( "Failed to parse stylesheet here: -> " << context.substr(0, 30) << "... <-" );
	}

	return false;
}

template bool parse_stylesheet<boost::spirit::istream_iterator>(boost::spirit::istream_iterator first, boost::spirit::istream_iterator last, Rules& s);
template bool parse_stylesheet<std::string::const_iterator>(std::string::const_iterator first, std::string::const_iterator last, Rules& s);

} // namespace stylesheets

bool Stylesheet::loadFile( const std::string &filename, const std::string &referer ) {
	if(filename.size() < 1){
		DS_LOG_WARNING("Blank stylesheet refferred from " << referer);
		return false;
	}
	mReferer = referer;
	namespace spirit = boost::spirit;

	// open file, disable skipping of whitespace
	std::ifstream in( filename );
	in.unsetf(std::ios::skipws);

	// wrap istream into iterator
	spirit::istream_iterator begin(in);
	spirit::istream_iterator end;

	// Parse the file into a Stylesheet
	return stylesheets::parse_stylesheet( begin, end, mRules );
}

}} // namespace ds::ui
