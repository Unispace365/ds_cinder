#pragma once
#ifndef DS_RAW_DATA_BUFFER_H
#define DS_RAW_DATA_BUFFER_H

namespace ds {

class RawDataBuffer
{
  public:
    RawDataBuffer(const unsigned &size = 0);
    ~RawDataBuffer();

    void        resize(unsigned size);
    char       *ptr() const;
    const char *constPtr() const;
    unsigned    size();
  protected:
    void        grow(unsigned size);

    char       *mBuffer;
    unsigned    mSize;
};

}

#endif//DS_RAW_DATA_BUFFER_H
