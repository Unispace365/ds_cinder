#include "stdafx.h"

#include "ds/data/font_list.h"

#include "ds/ui/sprite/sprite_engine.h"
#include "ds/ui/service/pango_font_service.h"

namespace ds {

namespace {
const int			EMPTY_ID(0);
const std::string	EMPTY_SZ("");
}

/**
 * ds::FontList
 */
FontList::FontList(ds::ui::SpriteEngine& eng)
	: mEngine(eng)
{
}

void FontList::clear(){
	mData.clear();
}

void FontList::installFont(const std::string& filePath, const std::string& fontName, const std::string& shortName){
	mEngine.getPangoFontService().loadFont(filePath, fontName);
	const int id = mData.size() + 1;
	mData[id] = Entry(filePath, fontName, shortName);
}

void FontList::registerFont(const std::string& fontName, const std::string& shortName){
	const int id = mData.size() + 1;
	mData[id] = Entry(fontName, fontName, shortName);
}

int FontList::getIdFromName(const std::string& n) const
{
	if(mData.empty()) return EMPTY_ID;
	for(auto it = mData.begin(), end = mData.end(); it != end; ++it) {
		if(it->second.mFilePath == n) return it->first;
		if(it->second.mShortName == n) return it->first;
	}
	return EMPTY_ID;
}

const std::string& FontList::getFilePathFromId(const int id) const {
	if(mData.empty()) return EMPTY_SZ;
	auto it = mData.find(id);
	if(it != mData.end()) {
		return it->second.mFilePath;
	}
	return EMPTY_SZ;
}

const std::string& FontList::getFontNameForShortName(const std::string& shortName) const{
	if(mData.empty()) return shortName;
	for(auto it = mData.begin(), end = mData.end(); it != end; ++it) {
		if(it->second.mShortName == shortName) return it->second.mFontName;
	}
	return shortName;
}

} // namespace ds
