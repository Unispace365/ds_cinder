#pragma once
#ifndef DS_READ_WRITE_H
#define DS_READ_WRITE_H

namespace ds {

class ReadWriteBuffer {
public:
	enum Postions {
		Begin,
		End
	};

	ReadWriteBuffer(size_t size = 0);
	~ReadWriteBuffer();

	bool read(char *buffer, size_t size);
	void rewindRead(size_t size);
	bool write(const char *buffer, size_t size);
	void rewindWrite(size_t size);

	void reserve(size_t size);
	void clear();
	size_t size();

	size_t getReadPosition() const;
	void setReadPosition(const size_t &position);
	void setReadPosition(const Postions &position);

	size_t getWritePosition() const;
	void setWritePosition(const size_t &position);
	void setWritePosition(const Postions &position);
private:
	void grow(size_t size);

	char *mBuffer;
	size_t mSize;
	size_t mBufferReadPosition;
	size_t mBufferWritePosition;
	size_t mMaxBufferWritePosition;
};

} // namespace ds

#endif//DS_READ_WRITE_H
