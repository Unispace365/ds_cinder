#include "stdafx.h"

#include "read_write_buffer.h"
#include <cstring>
#include "ds\math\math_func.h"

namespace ds {

ReadWriteBuffer::ReadWriteBuffer(size_t size /*= 0*/)
	: mSize(size)
	, mBuffer(nullptr)
	, mBufferReadPosition(0)
	, mBufferWritePosition(0)
	, mMaxBufferWritePosition(0)
{
	if(mSize > 0) {
		mBuffer = new char[mSize];
		memset(mBuffer, 0, mSize);
	}
}

ReadWriteBuffer::~ReadWriteBuffer(){
	if(mBuffer) {
		delete[] mBuffer;
		mBuffer = nullptr;
	}
}

bool ReadWriteBuffer::read(char *buffer, size_t size){
	if(mBufferReadPosition + size > mMaxBufferWritePosition)
		return false;

	memcpy(buffer, mBuffer + mBufferReadPosition, size);
	mBufferReadPosition += size;

	return true;
}

bool ReadWriteBuffer::write(const char *buffer, size_t size){
	if(mBufferWritePosition + size > mSize)
		grow(math::getNextPowerOf2(static_cast<int32_t>(mSize + size)));

	memcpy(mBuffer + mBufferWritePosition, buffer, size);
	mBufferWritePosition += size;
	if(mBufferWritePosition > mMaxBufferWritePosition)
		mMaxBufferWritePosition = mBufferWritePosition;

	return true;
}

void ReadWriteBuffer::reserve(size_t size){
	if(mSize > size)
		return;

	grow(size);
}

void ReadWriteBuffer::clear()
{
	mBufferReadPosition = 0;
	mBufferWritePosition = 0;
	mMaxBufferWritePosition = 0;
}

size_t ReadWriteBuffer::size(){
	return mSize;
}

size_t ReadWriteBuffer::getReadPosition() const{
	return mBufferReadPosition;
}

void ReadWriteBuffer::setReadPosition(const size_t &position){
	if(position > mMaxBufferWritePosition) {
		mBufferReadPosition = mMaxBufferWritePosition;
	}

	mBufferReadPosition = position;
}

void ReadWriteBuffer::setReadPosition(const Postions &position){
	if(Begin == position)
		mBufferReadPosition = 0;
	else if(End == position)
		mBufferReadPosition = mMaxBufferWritePosition;
}

size_t ReadWriteBuffer::getWritePosition() const {
	return mBufferWritePosition;
}

void ReadWriteBuffer::setWritePosition(const size_t &position){
	if(position > mMaxBufferWritePosition) {
		mBufferWritePosition = mMaxBufferWritePosition;
	}

	mBufferWritePosition = position;
}

void ReadWriteBuffer::setWritePosition(const Postions &position){
	if(Begin == position)
		mBufferWritePosition = 0;
	else if(End == position)
		mBufferWritePosition = mMaxBufferWritePosition;
}

void ReadWriteBuffer::grow(size_t size){
	size_t newSize = size;
	char *newBuffer = new char[newSize];
	memset(newBuffer, 0, newSize);

	if(mBuffer) {
		memcpy(newBuffer, mBuffer, mSize);
		delete[] mBuffer;
		mBuffer = nullptr;
	}

	mBuffer = newBuffer;
	mSize = newSize;
}

void ReadWriteBuffer::rewindRead(size_t size){
	if(size > mBufferReadPosition)
		mBufferReadPosition = 0;
	else
		mBufferReadPosition -= size;
}

void ReadWriteBuffer::rewindWrite(size_t size){
	size_t newWriteBufferPosition = mBufferWritePosition;
	if(size > mBufferWritePosition)
		newWriteBufferPosition = 0;
	else
		newWriteBufferPosition -= size;

	if(mBufferWritePosition == mMaxBufferWritePosition)
		mMaxBufferWritePosition = newWriteBufferPosition;
	mBufferWritePosition = newWriteBufferPosition;
}

} // namespace ds
