#include "stdafx.h"

#include <string>

#include "ds/data/data_buffer.h"

namespace ds {

DataBuffer::DataBuffer(unsigned initialStreamSize)
  : mStream(initialStreamSize) {}

unsigned DataBuffer::size() {
	unsigned currentPosition = mStream.getReadPosition();

	mStream.setReadPosition(ReadWriteBuffer::End);
	unsigned length = mStream.getReadPosition();
	mStream.setReadPosition(currentPosition);

	return length;
}

void DataBuffer::seekBegin() {
	mStream.setReadPosition(ReadWriteBuffer::Begin);
	mStream.setWritePosition(ReadWriteBuffer::Begin);
}

void DataBuffer::clear() {
	mStream.clear();
}

void DataBuffer::addRaw(const char* b, unsigned size) {
	mStream.write(b, size);
}

bool DataBuffer::readRaw(char* b, unsigned size) {
	unsigned currentPosition = mStream.getReadPosition();

	mStream.setReadPosition(ReadWriteBuffer::End);
	unsigned length = mStream.getReadPosition();
	mStream.setReadPosition(currentPosition);

	if (size > (length - currentPosition)) return false;

	mStream.read(b, size);
	return true;
}

void DataBuffer::add(const char* b, unsigned size) {
	add(size);
	mStream.write(b, size);
}

void DataBuffer::add(const char* cs) {
	add<std::string>(cs);
}

void DataBuffer::add(const wchar_t* cs) {
	add<std::wstring>(cs);
}

bool DataBuffer::read(char* b, unsigned size) {
	auto wsize = read<unsigned>();
	if (wsize != size) {
		add(wsize);
		return false;
	}

	unsigned currentPosition = mStream.getReadPosition();

	mStream.setReadPosition(ReadWriteBuffer::End);
	unsigned length = mStream.getReadPosition();
	mStream.setReadPosition(currentPosition);

	if (size > (length - currentPosition)) return false;


	mStream.read(b, size);
	return true;
}


// Template specializations
template <>
void DataBuffer::add<std::string>(const std::string& t) {
	auto size = unsigned(t.size());
	add(size);
	mStream.write(t.c_str(), size);
}

template <>
void DataBuffer::add<std::wstring>(const std::wstring& t) {
	unsigned size = static_cast<unsigned int>(t.size()) * sizeof(wchar_t);
	add(size);
	mStream.write(reinterpret_cast<const char*>(t.c_str()), size);
}

template <>
std::string DataBuffer::read<std::string>() {
	auto	 size			 = read<unsigned>();
	unsigned currentPosition = mStream.getReadPosition();

	mStream.setReadPosition(ReadWriteBuffer::End);
	unsigned length = mStream.getReadPosition();
	mStream.setReadPosition(currentPosition);

	if (size > (length - currentPosition)) {
		add(size);
		return {};
	}

	mStringBuffer.setSize(size);

	mStream.read(mStringBuffer.data(), size);
	return std::string(mStringBuffer.data(), size);
}

template <>
std::wstring DataBuffer::read<std::wstring>() {
	auto	 size			 = read<unsigned>();
	unsigned currentPosition = mStream.getReadPosition();

	mStream.setReadPosition(ReadWriteBuffer::End);
	unsigned length = mStream.getReadPosition();
	mStream.setReadPosition(currentPosition);

	if (size > (length - currentPosition)) {
		add(size);
		return {};
	}

	mWStringBuffer.setSize(size);

	mStream.read(mWStringBuffer.data(), size);
	return std::wstring(reinterpret_cast<const wchar_t*>(mWStringBuffer.data()), size / 2);
}

} // namespace ds
