#include "stdafx.h"

#include "data_buffer.h"
#include <string>

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

  unsigned currentPosition = mStream.getReadPosition();

  mStream.setReadPosition(ReadWriteBuffer::End);
  unsigned length = mStream.getReadPosition();
  mStream.setReadPosition(currentPosition);

  if (size > (length - currentPosition))
    return false;


  mStream.read(b, size);
  return true;
}

DataBuffer::DataBuffer(unsigned initialStreamSize)
  : mStream(initialStreamSize)
{

}

void DataBuffer::seekBegin()
{
  mStream.setReadPosition(ReadWriteBuffer::Begin);
  mStream.setWritePosition(ReadWriteBuffer::Begin);
}

unsigned DataBuffer::size()
{
  unsigned currentPosition = mStream.getReadPosition();

  mStream.setReadPosition(ReadWriteBuffer::End);
  unsigned length = mStream.getReadPosition();
  mStream.setReadPosition(currentPosition);

  return length;
}

void DataBuffer::clear()
{
  mStream.clear();
}

void DataBuffer::addRaw( const char *b, unsigned size )
{
  mStream.write(b, size);
}

bool DataBuffer::readRaw( char *b, unsigned size )
{
  unsigned currentPosition = mStream.getReadPosition();

  mStream.setReadPosition(ReadWriteBuffer::End);
  unsigned length = mStream.getReadPosition();
  mStream.setReadPosition(currentPosition);

  if (size > (length - currentPosition))
    return false;

  mStream.read(b, size);
  return true;
}

} // namespace ds
