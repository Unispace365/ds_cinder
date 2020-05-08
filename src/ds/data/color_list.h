#pragma once

#include <string>
#include <unordered_map>
#include "cinder/Color.h"

namespace ds {

/**
* \class FontList
* \brief A list of colors. Used to create a central color pallet usable by layout xmls.
*/
class ColorList
{
public:
	ColorList();

	void					clear();
	bool					empty() const;

	/// Short name can be supplied by the app and used to refer to colors from now on.
	/// Often it might be something in a settings file.
	void					install(const ci::ColorA& color, const std::string& shortName = "");

	/// Answer > 0 for a valid ID. Name can either be the file or short name, which should never conflict.
	size_t 					getIdFromName(const std::string&) const;
	const ci::ColorA&		getColorFromId(const size_t id) const;

	/// Clients give either a shortname and I give them a color
	const ci::ColorA&		getColorFromName(const std::string&) const;
	const ci::ColorA&		getColorFromName(const std::wstring&) const;

	/// Returns a shortname if it matches the color. Returns an empty string otherwise
	std::string				getNameFromColor(const ci::ColorA&) const;

private:
	class Entry {
	public:
		Entry()           { }
		Entry(const ci::ColorA& color, const std::string& name) : mColor(color), mShortName(name) { }
		ci::ColorA		  mColor;
		std::string       mShortName;
	};
	std::unordered_map<size_t, Entry>	mData;
};

} // namespace ds
