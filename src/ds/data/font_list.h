#pragma once
#ifndef DS_DATA_FONTLIST_H_
#define DS_DATA_FONTLIST_H_

#include <string>
#include <unordered_map>

namespace ds {

/**
 * \class ds::FontList
 * \brief A list of fonts. This is used purely for performance between
 * server and client -- upon startup, an app can cache any fonts it wants here,
 * and they will be given a simple int to be used to refer to them instead of
 * shuttling down the full name any time a text sprite is created.
 */
class FontList
{
  public:
    FontList();

    void                clear();

    void                install(const std::string&);
    // Answer > 0 for a valid ID.
    int 						    getId(const std::string&) const;
    const std::string&  getName(const int id) const;

  private:
    std::unordered_map<std::string, int> 
                        mData;
};

} // namespace ds

#endif // DS_DATA_FONTLIST_H_