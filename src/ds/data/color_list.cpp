#include "stdafx.h"

#include "ds/data/color_list.h"

namespace ds {

	namespace {
		const int           EMPTY_ID(0);
		const std::string   EMPTY_SZ("");
		const ci::ColorA		EMPTY_COL(255, 0, 255, 255);
	}

	/**
	* ds::FontList
	*/
	ColorList::ColorList()
	{
	}

	void ColorList::clear()
	{
		mData.clear();
	}

	bool ColorList::empty() const{
		return (mData.empty());
	}


	void ColorList::install(const ci::ColorA& color , const std::string& shortName)
	{
		const int     id = mData.size() + 1;
		mData[id] = Entry(color, shortName);
	}

	int ColorList::getIdFromName(const std::string& n) const
	{
		if(mData.empty()) return EMPTY_ID;
		for(auto it = mData.begin(), end = mData.end(); it != end; ++it) {
			if(it->second.mShortName == n) return it->first;
		}
		return EMPTY_ID;
	}

	const ci::ColorA& ColorList::getColorFromId(const int id) const
	{
		if(mData.empty()) return EMPTY_COL;
		auto it = mData.find(id);
		if(it != mData.end()) {
			return it->second.mColor;
		}
		return EMPTY_COL;
	}

	const ci::ColorA& ColorList::getColorFromName(const std::string& n) const
	{
		if(mData.empty()) return EMPTY_COL;
		for(auto it = mData.begin(), end = mData.end(); it != end; ++it) {
			if(it->second.mShortName == n){
				return it->second.mColor;
			}
		}
		return EMPTY_COL;
	}

} // namespace ds
