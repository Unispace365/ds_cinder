#pragma once
#ifndef DS_DATA_BUFFER_H
#define DS_DATA_BUFFER_H
#include <string>
#include <sstream>

namespace ds {

class DataBuffer
{
  public:
    DataBuffer();
    const std::string &getSteam() const;
    void add(const char *b, unsigned size);
    template <typename T>
    void add(const T &t)
    {
      mStream.write((const char *)(&t), sizeof(t));
    }

    template <>
    void add<std::string>(const std::string &s);
    template <>
    void add<std::wstring>(const std::wstring &ws);
    void add(const char *cs);
    void add(const wchar_t *cs);


    bool read(char *b, unsigned size);
    template <typename T>
    T read()
    {
      T t;
      mStream.read((char *)(&t), sizeof(t));
      return t;
    }

    void seekBegin();

    template <>
    std::string read<std::string>();
    template <>
    std::wstring read<std::wstring>();

    template <typename T>
    void rewindRead()
    {
      std::streamoff currentPosition = mStream.tellg();

      if (currentPosition - sizeof(T) < 0)
        return;
      mStream.seekg(currentPosition - sizeof(T));
    }

    template <typename T>
    void rewindAdd()
    {
      std::streamoff currentPosition = mStream.tellp();

      if (currentPosition - sizeof(T) < 0)
        return;
      mStream.seekp(currentPosition - sizeof(T));
    }
  private:
    std::stringstream mStream;
};

template <>
std::string DataBuffer::read<std::string>()
{
  unsigned size = read<unsigned>();
  std::streamoff currentPosition = mStream.tellg();

  mStream.seekg(0, std::stringstream::end);
  std::streamoff length = mStream.tellg();
  mStream.seekg(currentPosition);

  if (size > (unsigned)(length - currentPosition)) {
    add(size);
    return std::string();
  }

  static unsigned bytesSize = size;
  static char *bytes = new char[bytesSize];

  if (bytesSize < size || !bytes) {
    if (bytes)
      delete []bytes;
    bytesSize = size;
    bytes = new char[bytesSize];
  }

  mStream.read(bytes, size);
  return std::string(bytes, size);
}

template <>
std::wstring DataBuffer::read<std::wstring>()
{
  unsigned size = read<unsigned>();
  std::streamoff currentPosition = mStream.tellg();

  mStream.seekg(0, std::stringstream::end);
  std::streamoff length = mStream.tellg();
  mStream.seekg(currentPosition);

  if (size > (unsigned)(length - currentPosition)) {
    add(size);
    return std::wstring();
  }

  static unsigned bytesSize = size;
  static wchar_t *bytes = new wchar_t[bytesSize];

  if (bytesSize < size || !bytes) {
    if (bytes)
      delete []bytes;
    bytesSize = size;
    bytes = new wchar_t[bytesSize];
  }

  mStream.read((char *)bytes, size);
  return std::wstring(bytes, size/2);
}

}

#endif//DS_DATA_BUFFER_H
