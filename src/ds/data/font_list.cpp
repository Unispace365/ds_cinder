#include "stdafx.h"

#include "ds/data/font_list.h"

namespace ds {

namespace {
const int           EMPTY_ID(0);
const std::string   EMPTY_SZ("");
}

/**
 * ds::FontList
 */
FontList::FontList()
{
}

void FontList::clear()
{
  mData.clear();
}

void FontList::install(const std::string& fileName, const std::string& shortName)
{
  const int     id = mData.size() + 1;
  mData[id] = Entry(fileName, shortName);
}

int FontList::getIdFromName(const std::string& n) const
{
  if (mData.empty()) return EMPTY_ID;
  for (auto it=mData.begin(), end=mData.end(); it != end; ++it) {
    if (it->second.mFileName == n) return it->first;
    if (it->second.mShortName == n) return it->first;
  }
  return EMPTY_ID;
}

const std::string& FontList::getFileNameFromId(const int id) const
{
  if (mData.empty()) return EMPTY_SZ;
  auto it = mData.find(id);
  if (it != mData.end()) {
    return it->second.mFileName;
  }
  return EMPTY_SZ;
}

const std::string& FontList::getFileNameFromName(const std::string& n) const
{
  if (mData.empty()) return n;
  for (auto it=mData.begin(), end=mData.end(); it != end; ++it) {
    if (it->second.mShortName == n) return it->second.mFileName;
  }
  return n;
}

} // namespace ds
