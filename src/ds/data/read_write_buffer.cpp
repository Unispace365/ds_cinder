#include "stdafx.h"

#include "ds/math/math_func.h"
#include "read_write_buffer.h"
#include <cstring>

namespace ds {

ReadWriteBuffer::ReadWriteBuffer(unsigned size /*= 0*/)
  : mSize(size)
  , mBuffer(nullptr)
  , mBufferReadPosition(0)
  , mBufferWritePosition(0)
  , mMaxBufferWritePosition(0) {
	if (mSize > 0) {
		mBuffer = new char[mSize];
		memset(mBuffer, 0, mSize);
	}
}

ReadWriteBuffer::~ReadWriteBuffer() {
	if (mBuffer) {
		delete[] mBuffer;
		mBuffer = nullptr;
	}
}

bool ReadWriteBuffer::read(char* buffer, unsigned size) {
	if (mBufferReadPosition + size > mMaxBufferWritePosition) return false;

	memcpy(buffer, mBuffer + mBufferReadPosition, size);
	mBufferReadPosition += size;

	return true;
}

bool ReadWriteBuffer::write(const char* buffer, unsigned size) {
	if (mBufferWritePosition + size > mSize) grow(math::getNextPowerOf2(static_cast<int32_t>(mSize + size)));

	memcpy(mBuffer + mBufferWritePosition, buffer, size);
	mBufferWritePosition += size;
	if (mBufferWritePosition > mMaxBufferWritePosition) mMaxBufferWritePosition = mBufferWritePosition;

	return true;
}

void ReadWriteBuffer::reserve(unsigned size) {
	if (mSize > size) return;

	grow(size);
}

void ReadWriteBuffer::clear() {
	mBufferReadPosition		= 0;
	mBufferWritePosition	= 0;
	mMaxBufferWritePosition = 0;
}

unsigned ReadWriteBuffer::size() {
	return mSize;
}

unsigned ReadWriteBuffer::getReadPosition() const {
	return mBufferReadPosition;
}

void ReadWriteBuffer::setReadPosition(const unsigned& position) {
	if (position > mMaxBufferWritePosition) {
		mBufferReadPosition = mMaxBufferWritePosition;
	}

	mBufferReadPosition = position;
}

void ReadWriteBuffer::setReadPosition(const Postions& position) {
	if (Begin == position)
		mBufferReadPosition = 0;
	else if (End == position)
		mBufferReadPosition = mMaxBufferWritePosition;
}

unsigned ReadWriteBuffer::getWritePosition() const {
	return mBufferWritePosition;
}

void ReadWriteBuffer::setWritePosition(const unsigned& position) {
	if (position > mMaxBufferWritePosition) {
		mBufferWritePosition = mMaxBufferWritePosition;
	}

	mBufferWritePosition = position;
}

void ReadWriteBuffer::setWritePosition(const Postions& position) {
	if (Begin == position)
		mBufferWritePosition = 0;
	else if (End == position)
		mBufferWritePosition = mMaxBufferWritePosition;
}

void ReadWriteBuffer::grow(unsigned size) {
	unsigned newSize   = size;
	char*	 newBuffer = new char[newSize];
	memset(newBuffer, 0, newSize);

	if (mBuffer) {
		memcpy(newBuffer, mBuffer, mSize);
		delete[] mBuffer;
		mBuffer = nullptr;
	}

	mBuffer = newBuffer;
	mSize	= newSize;
}

void ReadWriteBuffer::rewindRead(unsigned size) {
	if (size > mBufferReadPosition)
		mBufferReadPosition = 0;
	else
		mBufferReadPosition -= size;
}

void ReadWriteBuffer::rewindWrite(unsigned size) {
	unsigned newWriteBufferPosition = mBufferWritePosition;
	if (size > mBufferWritePosition)
		newWriteBufferPosition = 0;
	else
		newWriteBufferPosition -= size;

	if (mBufferWritePosition == mMaxBufferWritePosition) mMaxBufferWritePosition = newWriteBufferPosition;
	mBufferWritePosition = newWriteBufferPosition;
}

} // namespace ds
