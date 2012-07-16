#pragma once
#ifndef DS_READ_WRITE_H
#define DS_READ_WRITE_H

namespace ds {

class ReadWriteBuffer
{
  public:
    enum Postions
    {
      Begin,
      End
    };

    ReadWriteBuffer(unsigned size = 0);
    ~ReadWriteBuffer();

    bool read(char *buffer, unsigned size);
    void rewindRead(unsigned size);
    bool write(const char *buffer, unsigned size);
    void rewindWrite(unsigned size);

    void reserve(unsigned size);
    void clear();
    unsigned size();

    unsigned getReadPosition() const;
    void setReadPosition(const unsigned &position);
    void setReadPosition(const Postions &position);

    unsigned getWritePosition() const;
    void setWritePosition(const unsigned &position);
    void setWritePosition(const Postions &position);
  private:
    void grow(unsigned size);
    unsigned getNextPowerOf2(unsigned number);

    char *mBuffer;
    unsigned mSize;
    unsigned mBufferReadPosition;
    unsigned mBufferWritePosition;
    unsigned mMaxBufferWritePosition;
};

} // namespace ds

#endif//DS_READ_WRITE_H
