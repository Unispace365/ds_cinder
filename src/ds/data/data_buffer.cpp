#include "stdafx.h"

#include "data_buffer.h"
#include <string>

namespace ds {

template <>
void DataBuffer::add<std::string>(const std::string &s){
	size_t size = (unsigned)s.size();
	add(size);
	mStream.write(s.c_str(), size);
}

void DataBuffer::add(const char *cs){
	add<std::string>(cs);
}

void DataBuffer::add(const wchar_t *cs){
	add<std::wstring>(cs);
}

template <>
void DataBuffer::add<std::wstring>(const std::wstring &ws){
	size_t size = (ws.size())*sizeof(wchar_t);
	add(size);
	mStream.write((const char *)(ws.c_str()), size);
}

void DataBuffer::add(const char *b, size_t size){
	add(size);
	mStream.write(b, size);
}

bool DataBuffer::read(char *b, size_t size){
	size_t wsize = read<size_t>();
	if(wsize != size) {
		add(wsize);
		return false;
	}

	size_t currentPosition = mStream.getReadPosition();

	mStream.setReadPosition(ReadWriteBuffer::End);
	size_t length = mStream.getReadPosition();
	mStream.setReadPosition(currentPosition);

	if(size > (length - currentPosition))
		return false;


	mStream.read(b, size);
	return true;
}

DataBuffer::DataBuffer(size_t initialStreamSize)
	: mStream(initialStreamSize)
{

}

void DataBuffer::seekBegin()
{
	mStream.setReadPosition(ReadWriteBuffer::Begin);
	mStream.setWritePosition(ReadWriteBuffer::Begin);
}

size_t DataBuffer::size()
{
	size_t currentPosition = mStream.getReadPosition();

	mStream.setReadPosition(ReadWriteBuffer::End);
	size_t length = mStream.getReadPosition();
	mStream.setReadPosition(currentPosition);

	return length;
}

void DataBuffer::clear()
{
	mStream.clear();
}

void DataBuffer::addRaw(const char *b, size_t size)
{
	mStream.write(b, size);
}

bool DataBuffer::readRaw(char *b, size_t size)
{
	size_t currentPosition = mStream.getReadPosition();

	mStream.setReadPosition(ReadWriteBuffer::End);
	size_t length = mStream.getReadPosition();
	mStream.setReadPosition(currentPosition);

	if(size > (length - currentPosition))
		return false;

	mStream.read(b, size);
	return true;
}

} // namespace ds
