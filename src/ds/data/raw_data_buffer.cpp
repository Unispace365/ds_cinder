#include "raw_data_buffer.h"
#include <cstring>

namespace ds {

RawDataBuffer::RawDataBuffer( const unsigned &size /*= 0*/ )
  : mSize(0)
  , mBuffer(nullptr)
{
  grow(size);
}

RawDataBuffer::~RawDataBuffer()
{
  if (mBuffer) {
    delete [] mBuffer;
    mBuffer = nullptr;
  }
}

void RawDataBuffer::resize( unsigned size )
{
  grow(size);
}

char *RawDataBuffer::ptr() const
{
  return mBuffer;
}

const char *RawDataBuffer::constPtr() const
{
  return mBuffer;
}

unsigned RawDataBuffer::size()
{
  return mSize;
}

void RawDataBuffer::grow( unsigned size )
{
  if (mSize >= size)
    return;

  unsigned newSize = size;
  char *newBuffer = new char[newSize];
  memset(newBuffer, 0, newSize);

  if (mBuffer) {
    memcpy(newBuffer, mBuffer, mSize);
    delete [] mBuffer;
    mBuffer = nullptr;
  }

  mBuffer = newBuffer;
  mSize = newSize;
}

}
