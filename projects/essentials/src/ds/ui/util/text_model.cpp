#include "stdafx.h"

#include "text_model.h"

#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeParser.h>

#include <ds/app/environment.h>
#include <ds/util/string_util.h>
#include <ds/content/content_model.h>


namespace ds {
namespace ui {

std::string processTextModel(const std::string& format, ds::model::ContentModelRef& model) {
	std::string output;
	auto		paired = ds::extractPairs(format, "{", "}");

	// any section within '{}' will be replaced with the value from the content model
	for (const auto& elemPair : paired) {
		const bool		   isInside = static_cast<bool>(elemPair.first);
		const std::string& theStr   = elemPair.second;

		if (isInside) {
			output.append(model.getPropertyString(theStr));
		} else {
			output.append(theStr);
		}
	}

	// Any Section in 'fn(funcationName,value)' will call the selected function
	// on the text
	paired = ds::extractPairs(output, "fn(", ")");
	output = "";
	for (const auto& elemPair : paired) {
		const bool		   isInside = static_cast<bool>(elemPair.first);
		const std::string& theStr   = elemPair.second;

		if (isInside) {
			auto fnSplit = ds::split(theStr, ",", true);
			output.append(processTextFunction(fnSplit));
		} else {
			output.append(theStr);
		}
	}

	return output;
}

std::string processTextFunction(std::vector<std::string>& fnSplit) {
	if (fnSplit.empty()) return std::string();

	std::string output;
	const auto& fnName = fnSplit[0];

	if (fnSplit.size() == 2) {
		if (fnName == "upper") {
			auto uped = fnSplit[1];
			std::transform(uped.begin(), uped.end(), uped.begin(), ::toupper);
			output.append(uped);
		} else if (fnName == "upper") {
			auto uped = fnSplit[1];
			std::transform(uped.begin(), uped.end(), uped.begin(), ::tolower);
			output.append(uped);
		}
	} else if (fnSplit.size() == 3 || fnSplit.size() == 4) {
		if (fnName == "utc_format" || fnName == "utc") {
			auto		   value	= fnSplit[1];
			auto		   outFmt   = fnSplit[2];
			auto		   parseFmt = (fnSplit.size() == 3) ? "" : fnSplit[3];

			output.append(utcFormat(value, outFmt, parseFmt));
		} else if (fnName == "utc_format_local" || fnName == "utc_local") {
			auto		   value	= fnSplit[1];
			auto		   outFmt   = fnSplit[2];
			auto		   parseFmt = (fnSplit.size() == 3) ? "" : fnSplit[3];

			output.append(utcFormat(value, outFmt, parseFmt, true));
		}
	}

	return output;
}

std::string utcFormat(const std::string& value, const std::string& outFmt, const std::string& parseFmt, const bool isLocal) {
	std::string	output;
	Poco::DateTime date;
	Poco::LocalDateTime localDate;
	int			   tzd = 0;

	if (value == "now" && !isLocal) {
		output = Poco::DateTimeFormatter::format(date, outFmt);
	} else if (value == "now" && isLocal) {
		output = Poco::DateTimeFormatter::format(localDate, outFmt);
	} else {
		if (!parseFmt.empty() && Poco::DateTimeParser::tryParse(parseFmt, value, date, tzd)) {
			if(isLocal){
				date.makeLocal(localDate.tzd());
			}
			output = Poco::DateTimeFormatter::format(date, outFmt);
		}else if (parseFmt.empty() && Poco::DateTimeParser::tryParse(value, date, tzd)) {
			if(isLocal){
				date.makeLocal(localDate.tzd());
			}
			output = Poco::DateTimeFormatter::format(date, outFmt);
		} else {
			output = value;
		}
	}

	return output;
}

}  // namespace ui
}  // namespace ds
