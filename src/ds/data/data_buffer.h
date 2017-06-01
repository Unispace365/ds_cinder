#pragma once
#ifndef DS_DATA_BUFFER_H
#define DS_DATA_BUFFER_H
#include <string>
#include "read_write_buffer.h"
#include "ds/query/recycle_array.h"

namespace ds {

/*
	* brief
	*   add functions must be matched be read function.
	*/
class DataBuffer
{
public:
	DataBuffer(size_t initialStreamSize = 0);
	size_t size();
	void seekBegin();
	void clear();

	template <typename T>
	bool canRead();

	// function to add raw data no size added.
	void addRaw(const char *b, size_t size);
	// function to read raw data no size will be read.
	bool readRaw(char *b, size_t size);

	// will write size when writing data.
	void add(const char *b, size_t size);
	void add(const char *cs);
	void add(const wchar_t *cs);
	template <typename T>
	void add(const T &t);

	// will read size from buffer and only read if size is available.
	bool read(char *b, size_t size);
	template <typename T>
	T read();

	template <typename T>
	void rewindRead();

	template <typename T>
	void rewindAdd();
private:
	ReadWriteBuffer		mStream;
	RecycleArray<char>	mStringBuffer;
	RecycleArray<char>	mWStringBuffer;
};

template <typename T>
bool ds::DataBuffer::canRead(){
	size_t size = sizeof(T);
	size_t currentPosition = mStream.getReadPosition();

	mStream.setReadPosition(ReadWriteBuffer::End);
	size_t length = mStream.getReadPosition();
	mStream.setReadPosition(currentPosition);

	if(size > (length - currentPosition))
		return false;
	return true;
}

template <typename T>
void ds::DataBuffer::add(const T &t){
	mStream.write((const char *)(&t), sizeof(t));
}

template <typename T>
T ds::DataBuffer::read(){
	T t;
	mStream.read((char *)(&t), sizeof(t));
	return t;
}

template <typename T>
void ds::DataBuffer::rewindRead(){
	size_t currentPosition = mStream.getReadPosition();

	if(sizeof(T) > currentPosition)
		return;

	mStream.rewindRead(sizeof(T));
}

template <typename T>
void ds::DataBuffer::rewindAdd(){
	size_t currentPosition = mStream.getWritePosition();

	if(sizeof(T) > currentPosition)
		return;

	mStream.rewindWrite(sizeof(T));
}

// Template specializations
template <>
void DataBuffer::add<std::string>(const std::string &s);
template <>
void DataBuffer::add<std::wstring>(const std::wstring &ws);

template <>
std::string DataBuffer::read<std::string>();
template <>
std::wstring DataBuffer::read<std::wstring>();

}

#endif//DS_DATA_BUFFER_H
