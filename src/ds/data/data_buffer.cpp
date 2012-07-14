#include "data_buffer.h"

namespace ds {

template <>
void DataBuffer::add<std::string>( const std::string &s )
{
  unsigned size = s.size();
  add(size);
  mStream.write(s.c_str(), size);
}

void DataBuffer::add(const char *cs)
{
  add<std::string>(cs);
}

void DataBuffer::add(const wchar_t *cs)
{
  add<std::wstring>(cs);
}

template <>
void DataBuffer::add<std::wstring>( const std::wstring &ws )
{
  unsigned size = (ws.size())*sizeof(wchar_t);
  add(size);
  mStream.write((const char *)(ws.c_str()), size);
}

void DataBuffer::add( const char *b, unsigned size )
{
  add(size);
  mStream.write(b, size);
}

bool DataBuffer::read( char *b, unsigned size )
{
  unsigned wsize = read<unsigned>();
  if (wsize != size) {
    add(wsize);
    return false;
  }

  std::streamoff currentPosition = mStream.tellg();

  mStream.seekg(std::stringstream::end);
  std::streamoff length = mStream.tellg();
  mStream.seekg(currentPosition);

  if (size > (unsigned)(length - currentPosition))
    return false;


  mStream.read(b, size);
  return true;
}

DataBuffer::DataBuffer()
{

}

void DataBuffer::seekBegin()
{
  mStream.seekg(std::stringstream::beg);
  mStream.seekp(std::stringstream::beg);
}

} // namespace ds
