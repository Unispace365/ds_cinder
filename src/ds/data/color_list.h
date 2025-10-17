#pragma once

#include "cinder/Color.h"
#include <string>
#include <unordered_map>

namespace ds {

class ColorList {
  public:
	ColorList();

	void clear();
	bool empty() const;

	/// Returns true if the color is installed
	bool hasColor(const std::string& shortName) const { return mData.find(shortName) != mData.end(); }

	/// Short name can be supplied by the app and used to refer to colors from now on.
	/// Often it might be something in a settings file.
	void install(const ci::ColorA& color, const std::string& shortName);

	/// Short name can be supplied by the app and used to refer to colors from now on.
	/// Often it might be something in a settings file
	void installOnce(const ci::ColorA& color, const std::string& shortName) {
		if (!hasColor(shortName)) {
			install(color, shortName);
		}
	}

	/// Clients give either a shortname and I give them a color
	const ci::ColorA& getColorFromName(const std::string&) const;
	const ci::ColorA& getColorFromName(const std::wstring&) const;

	/// Returns a shortname if it matches the color. Returns an empty string otherwise
	std::string getNameFromColor(const ci::ColorA&) const;

  private:
	std::unordered_map<std::string, ci::ColorA> mData;
};

} // namespace ds
