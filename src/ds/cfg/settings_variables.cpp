#include "stdafx.h"

#include "settings_variables.h"

#include <ds/app/engine/engine.h>
#include <ds/app/engine/engine_cfg.h>

#include "ds/math/fparser.hh"


namespace {

static std::unordered_map<std::string, std::string> VARIABLE_MAP;

class Init {
  public:
	Init() {
		ds::App::AddServerSetup([](ds::Engine& e) {
			VARIABLE_MAP.clear();

			VARIABLE_MAP["world_width"]	 = std::to_string(e.getWorldWidth());
			VARIABLE_MAP["world_height"] = std::to_string(e.getWorldHeight());
			VARIABLE_MAP["world_size"]	 = ds::unparseVector(ci::vec2(e.getWorldWidth(), e.getWorldHeight()));
			VARIABLE_MAP["anim_dur"]	 = std::to_string(e.getAnimDur());

			e.getAppSettings().forEachSetting([](const ds::cfg::Settings::Setting& theSetting) {
				
				ds::cfg::SettingsVariables::addVariable(theSetting.mName, theSetting.mRawValue);
			});
			e.getAppSettings().replaceSettingVariablesAndExpressions();
			e.getWafflesSettings().forEachSetting([](const ds::cfg::Settings::Setting& theSetting) {
				std::string value = theSetting.mRawValue;
				if (!theSetting.mMultiplier.empty()) {
					value = ds::cfg::SettingsVariables::doMultiply(
						theSetting.mOriginalValue.empty() ? theSetting.mRawValue : theSetting.mOriginalValue,
						theSetting.mMultiplier, theSetting.mType);
					DS_LOG_INFO(std::string("Adding ") << theSetting.mName << " to variables with scaled value of "
													   << value << " (" << theSetting.mRawValue << ")");
				}
				ds::cfg::SettingsVariables::addVariable(theSetting.mName, value);
				
			});
			e.getWafflesSettings().replaceSettingVariablesAndExpressions();
		});
	}
	void doNothing() {}
};
Init INIT;


} // namespace

namespace ds::cfg {


std::string SettingsVariables::parseExpression(const std::string& theExpr) {

	std::string returny = theExpr;

	FunctionParser fparser;
	fparser.AddConstant("pi", 3.1415926535897932);
	int res = fparser.Parse(theExpr, "");
	if (res > -1) {
		DS_LOG_WARNING("SettingsVariables::parseExpression() error parsing: " << fparser.ErrorMsg());
		return "0.0";
	}

	double* vals	  = {};
	double	theResult = fparser.Eval(vals);
	returny			  = std::to_string(theResult);

	if (returny == "nan") {
		DS_LOG_WARNING("SettingsVariables: Experession didn't parse to a number! Using 0.0");
		return "0.0";
	}

	DS_LOG_VERBOSE(2, "SettingsVariables: Parsed expression: " << theExpr << " into " << returny);

	return returny;
}

std::string SettingsVariables::parseAllExpressions(const std::string& value) {
	// Do we have any wrapeped expressions?
	auto exprFindy = value.find("#expr{");

	if (exprFindy == std::string::npos) {
		// Or maybe we just are an expression?
		auto findy = value.find("#expr");
		if (findy != std::string::npos) {
			auto theExpr = value.substr(findy + 5);
			return parseExpression(theExpr);
		}
		return value;
	}

	std::string finalValue;

	// Splits value string into a vector of (int, string) pairs
	// anything between '#expr{' and '}' will be a pair of (1, str),
	// and non-expr parts of the string will be pairs of (0, str)
	for (const auto elemPair : ds::extractPairs(value, "#expr{", "}")) {
		bool			   isExpr = static_cast<bool>(elemPair.first);
		const std::string& val	  = elemPair.second;
		if (isExpr) {
			finalValue.append(parseExpression(val));
		} else {
			finalValue.append(val);
		}
	}

	// I don't think this should ever happen, but just in case we'll try to recurse?
	exprFindy	  = finalValue.find("#expr{");
	auto endFindy = finalValue.find("}");
	if (exprFindy != std::string::npos && endFindy == std::string::npos) {
		DS_LOG_WARNING("Syntax error parsing expression: missing } in " << value);
	} else if (exprFindy != std::string::npos) {
		finalValue = parseAllExpressions(finalValue);
	}
	return finalValue;
}

std::string SettingsVariables::replaceVariables(const std::string& value, const VariableMap& local_map) {
	if (value.find("$_") != std::string::npos) {
		/// keep track of parses, cause it could get circular
		unsigned int numTries = 0;

		auto theReplacement = value;
		int	 maxTries		= 100000;
		while (numTries < maxTries) {

			auto newReplacement = replaceSingleVariable(theReplacement, local_map);

			/// nothing was replaced, we're done
			if (newReplacement == theReplacement) {
				break;
			}

			theReplacement = newReplacement;

			/// No more parameters, skipsies
			if (newReplacement.find("$_") == std::string::npos) {
				break;
			}

			numTries++;
		}

		if (numTries == maxTries) {
			DS_LOG_WARNING("SettingsVariables::replaceVariables() tried max tries, that means you have a circular "
						   "references in your parameters");
		}

		return theReplacement;
	}
	return value;
}

std::string SettingsVariables::replaceSingleVariable(const std::string& value, const VariableMap& local_map) {

	auto theStart = value.find("$_");

	if (theStart == std::string::npos) return value;

	auto theEnd = value.find(" ", theStart);

	auto commaEnd = value.find(",", theStart);
	if (commaEnd < theEnd) theEnd = commaEnd;

	auto semiEnd = value.find(";", theStart);
	if (semiEnd < theEnd) theEnd = semiEnd;

	auto brackSta = value.find("{", theStart);
	if (brackSta < theEnd) theEnd = brackSta;

	auto brackEnd = value.find("}", theStart);
	if (brackEnd < theEnd) theEnd = brackEnd;

	auto period = value.find(".", theStart);
	if (period < theEnd) theEnd = period;

	auto quot = value.find("'", theStart);
	if (quot < theEnd) theEnd = quot;

	if (theEnd == std::string::npos) theEnd = value.size();

	auto beforeString = value.substr(0, theStart);
	auto paramName	  = value.substr(theStart + 2, theEnd - theStart - 2); // ditch the $_
	auto endString	  = value.substr(theEnd);

	// std::cout << "RSP: " << value << std::endl << "\tBEFORE:" << beforeString << std::endl <<  "\tPARAM:" <<
	// paramName << std::endl << "\tAFTER:" << endString << std::endl << "\tInd:" << theStart << " " << theEnd <<
	// std::endl;

	VariableMap& combined_map = VARIABLE_MAP;
	if (local_map.size() > 0) {
		combined_map = local_map;
	}

	auto findy = combined_map.find(paramName);
	if (findy != combined_map.end()) {
		std::string replacement = findy->second;

		return beforeString + replacement + endString;
	} else {
		DS_LOG_WARNING("SettingsVariables::replaceSingleVariable() parameter not found! Name=" << paramName);
		return beforeString + endString;
	}

	return value;
}

std::string SettingsVariables::doMultiply(const std::string& value, const std::string& multiplyKey, const std::string& type) {
	bool inverse = false;
	auto mkey	 = multiplyKey;
	//if the first character is an !, then set the inverse flag to true and remove the !
	if (mkey[0] == '!') {
		inverse = true;
		mkey	= mkey.substr(1);
	}

	VariableMap& combined_map = VARIABLE_MAP;
	//if (local_map.size() > 0) {
	//	combined_map = local_map;
	//}
	std::string val	  = value;
	auto findy = combined_map.find(mkey);
	if (findy != combined_map.end()) {
		float multiplier = ds::string_to_float(findy->second);
		
		//do multiplication for each of the following types int, float, double, vec2, vec3, rect
		if (type == "int") {
			int valInt = ds::string_to_int(val);
			if (inverse) {
				valInt /= multiplier;
			}
			else {
				valInt *= multiplier;
			}
			val = std::to_string(valInt);
		}
		else if (type == "float") {
			float valFloat = ds::string_to_float(val);
			if (inverse) {
				valFloat /= multiplier;
			}
			else {
				valFloat *= multiplier;
			}
			val = std::to_string(valFloat);
		}
		else if (type == "double") {
			double valDouble = ds::string_to_double(val);
			if (inverse) {
				valDouble /= multiplier;
			}
			else {
				valDouble *= multiplier;
			}
			val = std::to_string(valDouble);
		}
		else if (type == "vec2") {
			ci::vec2 valVec2 = ds::parseVector(val);
			if (inverse) {
				valVec2 /= multiplier;
			}
			else {
				valVec2 *= multiplier;
			}
			val = ds::unparseVector(valVec2);
		}
		else if (type == "vec3") {
			ci::vec3 valVec3 = ds::parseVector(val);
			if (inverse) {
				valVec3 /= multiplier;
			}
			else {
				valVec3 *= multiplier;
			}
			val = ds::unparseVector(valVec3);
		}
		else if (type == "rect") {
			ci::Rectf valRect = ds::parseRect(val);
			if (inverse) {
				valRect /= multiplier;
			}
			else {
				valRect *= multiplier;
			}
			val = ds::unparseRect(valRect);
		}
		else {
			return val;
		}
	} else {
		DS_LOG_WARNING("SettingsVariables::doMultiply() parameter not found! Name=" << mkey);
	}
	return val;
}

void SettingsVariables::addVariable(const std::string& varName, const std::string& varValue) {
	if (varName.empty()) {
		DS_LOG_WARNING("SettingsVariables: No variable name specified when adding variable");
		return;
	}

	VARIABLE_MAP[varName] = varValue;
}


VariableMap SettingsVariables::insertAppToLocal(VariableMap& local_map) {
	auto ret_val = VariableMap(local_map);
	ret_val.insert(VARIABLE_MAP.cbegin(), VARIABLE_MAP.cend());
	return ret_val;
}


} // namespace ds::cfg
