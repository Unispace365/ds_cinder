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

    // Short name can be supplied by the app and used to refer to fonts from now on.
    // Often it might be something in a settings file.
    void                install(const std::string& fileName, const std::string& shortName = "");
    // Answer > 0 for a valid ID. Name can either be the file or short name, which should never conflict.
    int 						    getIdFromName(const std::string&) const;
    const std::string&  getFileNameFromId(const int id) const;
    // Clients give either a filename or a shortname, and I answer with the resulting filename
    const std::string&  getFileNameFromName(const std::string&) const;

  private:
    class Entry {
    public:
      Entry()           { }
      Entry(const std::string& fileName, const std::string& shortName) : mFileName(fileName), mShortName(shortName) { }
      std::string       mFileName;
      std::string       mShortName;      
    };
    std::unordered_map<int, Entry> 
                        mData;
};

} // namespace ds

#endif // DS_DATA_FONTLIST_H_