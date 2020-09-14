#pragma once
#include "yoga/YGNode.h"
namespace ds::ui {
	
	class FlexboxParser
	{

	public:
		static bool parseProperty(std::string property, std::string value, YGNodeRef node);
		static std::tuple<std::string, bool> cleanValue(std::string value);
		
	private:
		static std::unordered_map < std::string, std::function<bool(YGNodeRef, std::string,bool)>> parsers; 
		
	};
}
