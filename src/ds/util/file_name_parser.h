#pragma once
#ifndef DS_UTIL_FILE_NAME_PARSER_H
#define DS_UTIL_FILE_NAME_PARSER_H
#include <string>
#include <exception>
#include "cinder/Vector.h"


namespace ds {

class ParseFileMetaException : public std::exception
{
  public:
    ParseFileMetaException(const std::string &metaValue);
    const char *what() const throw();
  private:
    std::string mWhat;
};

// throws ParseFileMetaException when parsing fails.
int parseFileMetaData(const std::string &filename, const std::string &metaValue);
// does not catch parseFileMetaData exception, so consider parseFileMetaDataSize throwing on failure.
ci::Vec2f parseFileMetaDataSize(const std::string &filename);

}

#endif//DS_UTIL_FILE_NAME_PARSER_H
