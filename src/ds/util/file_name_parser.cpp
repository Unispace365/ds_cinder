#include "file_name_parser.h"
#include "string_util.h"

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
    if (str.find(metaValue) != std::string::npos) {
      std::vector<std::string> splitMeta = split(str, "_");
      int val;
      if (!string_to_value(splitMeta.back(), val))
        throw ParseFileMetaException("Was unable to find meta data for: "+metaValue);
      return val;
    }
  }

  throw ParseFileMetaException("Was unable to find meta data for: "+metaValue);
}

ci::Vec2f parseFileMetaDataSize(const std::string &filename)
{
  int wVal = parseFileMetaData(filename, "w_");
  int hVal = parseFileMetaData(filename, "h_");

  return ci::Vec2f(static_cast<float>(wVal), static_cast<float>(hVal));
}

}