#include "file_name_parser.h"
#include "string_util.h"
#include <map>

namespace ds {

ParseFileMetaException::ParseFileMetaException( const std::string &metaValue )
  : mWhat(metaValue)
{

}

const char *ParseFileMetaException::what() const throw()
{
  return mWhat.c_str();
}

int parseFileMetaData( const std::string &filename, const std::string &metaValue )
{
  std::vector<std::string> splitString = ds::split(filename, ".");

  for ( auto it = splitString.begin(), it2 = splitString.end(); it != it2; ++it ) {
  	std::string str = *it;
    size_t pos = str.find(metaValue);
    if (pos != std::string::npos && pos == 0) {
      std::vector<std::string> splitMeta = split(str, "_");
      int val;
      if (!string_to_value(splitMeta.back(), val))
        throw ParseFileMetaException("Was unable to find meta data for: "+metaValue);
      return val;
    }
  }

  throw ParseFileMetaException("Was unable to find meta data for: "+metaValue);
}

static std::map<std::string, ci::Vec2f> ParsedAttribs;

ci::Vec2f parseFileMetaDataSize(const std::string &filename)
{
  std::map<std::string, ci::Vec2f>::iterator found = ParsedAttribs.find(filename);

  if (found != ParsedAttribs.end())
    return found->second;

  int wVal = parseFileMetaData(filename, "w_");
  int hVal = parseFileMetaData(filename, "h_");

  ci::Vec2f values(static_cast<float>(wVal), static_cast<float>(hVal));
  ParsedAttribs[filename] = values;

  return values;
}

}