#pragma once
#ifndef DS_DATA_FONTLIST_H_
#define DS_DATA_FONTLIST_H_

#include <string>
#include <unordered_map>

namespace ds {
namespace ui {
class SpriteEngine;
}

/**
 * \class FontList
 * \brief A list of fonts. This is used purely for performance between
 * server and client -- upon startup, an app can cache any fonts it wants here,
 * and they will be given a simple int to be used to refer to them instead of
 * shuttling down the full name any time a text sprite is created.
 */
class FontList {
public:
	FontList(ds::ui::SpriteEngine& eng);

	void				clear();

	/// Loads and registers a font to be used by the system. 
	/// \param filePath is the absolute path to the font file (otf or ttf generally). Something like c:/path/to/your/app/folder/data/fonts/notosans-bold.otf
	/// \param fontName is the Pango-recognized name of the font. This will be something like "Noto Sans Bold" or "Arial"
	/// \param shortName is the name for the font that you can set on a text sprite. mMyText->setFont("noto_sans_bold"). Note: this can be the same as the Pango fontName if you want. Defaults to shortName if blank
	void				installFont(const std::string& filePath, const std::string& fontName, const std::string& shortName = "");

	/// Does the above, but doesn't load the font file. This is for fonts that are already installed on a system-level
	void				registerFont(const std::string& fontName, const std::string& shortName);

	/// Answer > 0 for a valid ID. Name can either be the file path, font name, or short name. Jebus help you if you name fonts the same thing.
	size_t				getIdFromName(const std::string&) const;
	const std::string&	getFilePathFromId(const size_t id) const;

	/// Clients give either a filename or a shortname, and I answer with the resulting font name to actually load in Pango.
	/// For example, pass in your custom name of "noto-sans-bold" or "the-title-font" and it will respond with the registered or installed mapped font name such as "Noto Sans Bold" or "Arial"
	/// If you haven't registered this shortname, it will be returned. So you can use system-installed fonts without having to do anything
	const std::string&	getFontNameForShortName(const std::string& shortName) const;

private:
	class Entry {
	public:
		Entry(){}
		Entry(const std::string& filePath, const std::string& fontName, const std::string& shortName) : mFilePath(filePath), mFontName(fontName), mShortName(shortName) { }
		std::string						mFilePath;
		std::string						mFontName;
		std::string						mShortName;
	};
	std::unordered_map<size_t, Entry>	mData;

	ds::ui::SpriteEngine&				mEngine;
};

} // namespace ds

#endif // DS_DATA_FONTLIST_H_
