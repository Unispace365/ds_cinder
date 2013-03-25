#include "ds/data/font_list.h"

namespace ds {

namespace {
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

void FontList::install(const std::string& fontname)
{
  const int     id = mData.size();
  mData[fontname] = id+1;
}

int FontList::getId(const std::string& fontname) const
{
  if (mData.empty()) return 0;
  auto it = mData.find(fontname);
  if (it != mData.end()) {
    return it->second;
  }
  return 0;
}

const std::string& FontList::getName(const int id) const
{
  if (mData.empty()) return EMPTY_SZ;
  for (auto it=mData.begin(), end=mData.end(); it != end; ++it) {
    if (it->second == id) return it->first;
  }
  return EMPTY_SZ;
}

} // namespace ds
