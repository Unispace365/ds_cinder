#include "stdafx.h"

#include "packet_chunker.h"
#include "snappy.h"
#include <iostream>
#include <algorithm>

namespace ds {
namespace net {

Chunker::Chunker()
	: mChunkSize(1400)
{
}

void Chunker::Chunkify(const char *src, unsigned size, unsigned groupId, std::vector<std::string> &dst){

	unsigned chunkSize = mChunkSize - sizeof(ChunkHeader);
	unsigned pos = 0;

	snappy::Compress(src, size, &mCompressedBuffer);

	unsigned nSize = mCompressedBuffer.size();
	unsigned numIterations = nSize / chunkSize;

	unsigned excess = nSize % chunkSize;

	unsigned total = numIterations + (excess ? 1 : 0);

	dst.resize(total);

	ChunkHeader header = { groupId, nSize, 0, total, chunkSize };

	int i;
	for(i = 0; i < numIterations; ++i){
		header.mId = i;
		dst[i].resize(sizeof(ChunkHeader) + chunkSize);
		std::copy(reinterpret_cast<char *>(&header), reinterpret_cast<char *>(&header) + sizeof(ChunkHeader), dst[i].begin());
		std::copy(mCompressedBuffer.begin() + pos, mCompressedBuffer.begin() + pos + chunkSize, dst[i].begin() + sizeof(ChunkHeader));
		pos += chunkSize;
	}

	if(excess){
		header.mId = i;
		dst[i].resize(sizeof(ChunkHeader) + excess);
		std::copy(reinterpret_cast<char *>(&header), reinterpret_cast<char *>(&header) + sizeof(ChunkHeader), dst[i].begin());
		std::copy(mCompressedBuffer.begin() + pos, mCompressedBuffer.begin() + pos + excess, dst[i].begin() + sizeof(ChunkHeader));
		pos += chunkSize;
	}
}

void Chunker::Chunkify(std::string src, unsigned groupId, std::vector<std::string> &dst){
	Chunkify(src.c_str(), src.size(), groupId, dst);
}


DeChunker::DeChunker(){
	mMaxReceivedSize = 1000;
}

bool DeChunker::addChunk(std::string &chunk){
	return addChunk(chunk.c_str(), chunk.size());
}

bool DeChunker::addChunk(const char *chunk, unsigned size){
	if(size < sizeof(ChunkHeader)){
		return false;
	}

	ChunkHeader chunkHeader;
	memcpy(reinterpret_cast<char *>(&chunkHeader), chunk, sizeof(ChunkHeader));

	if(chunkHeader.mId == 0 && chunkHeader.mGroupId == 0){
		clearReceived();
	}

	if(std::find(mReceived.begin(), mReceived.end(), chunkHeader.mGroupId) != mReceived.end()
	   && std::find(mGroupsReceived.begin(), mGroupsReceived.end(), chunkHeader.mGroupId) == mGroupsReceived.end()){
		return false;
	}

	auto found = mDataChunks.find(chunkHeader.mGroupId);

	auto &stats = mDataChunks[chunkHeader.mGroupId];
	if(found == mDataChunks.end()){

		for(unsigned i = 0; i < chunkHeader.mTotal; ++i){
			stats.mIdsMissing.push_back(i);
		}

		if(!mReserveStrings.empty()) {
			stats.mData = std::move(mReserveStrings.back());
			mReserveStrings.pop_back();
		} else {
			stats.mData = std::unique_ptr<std::string>(new std::string);
		}

		if(stats.mData.get()){
			stats.mData.get()->resize(chunkHeader.mSize);
		}

		mGroupsReceived.push_back(chunkHeader.mGroupId);
		mReceived.push_back(chunkHeader.mGroupId);

		while(mReceived.size() > mMaxReceivedSize){
			mReceived.pop_front();
		}

		std::sort(mGroupsReceived.begin(), mGroupsReceived.end(), [](unsigned a, unsigned b) -> bool {
			if(a > b)
				return true;
			return false;
		});
	}

	addChunkToGroup(stats, chunk, size);

	if(stats.mIdsMissing.empty()){
		mGroupsAvailable.push_back(chunkHeader.mGroupId);
		std::sort(mGroupsAvailable.begin(), mGroupsAvailable.end(), [](unsigned a, unsigned b) -> bool {
			if(a > b){
				return true;
			}
			return false;
		});
		return true;
	}
	return false;
}

void DeChunker::addChunkToGroup(DeChunkStats &stats, const char *chunk, unsigned size){
	ChunkHeader chunkHeader;
	memcpy(reinterpret_cast<char *>(&chunkHeader), chunk, sizeof(ChunkHeader));

	auto found = std::find(stats.mIdsMissing.begin(), stats.mIdsMissing.end(), chunkHeader.mId);
	if(found == stats.mIdsMissing.end())
		return;

	unsigned nSize = chunkHeader.mSize / chunkHeader.mChunkSize;
	unsigned pos = chunkHeader.mId * chunkHeader.mChunkSize;

	if(stats.mData.get()){
		std::copy(chunk + sizeof(ChunkHeader), chunk + size, stats.mData.get()->begin() + pos);
	}
	stats.mIdsMissing.remove(chunkHeader.mId);
}

bool DeChunker::getNextGroup(std::string &dst){
	if(!mGroupsAvailable.empty() && !mGroupsReceived.empty())	{

		unsigned groupId = mGroupsAvailable.back();
		
		auto gr = std::find(mGroupsReceived.begin(), mGroupsReceived.end(), groupId);
		if(gr != mGroupsReceived.end()){
			mGroupsAvailable.pop_back();
			mGroupsReceived.erase(gr);

			if(mDataChunks[groupId].mData.get()){
				snappy::Uncompress(mDataChunks[groupId].mData.get()->c_str(), mDataChunks[groupId].mData.get()->size(), &dst);
			}

			mReserveStrings.push_back(std::move(mDataChunks[groupId].mData));
			mDataChunks.erase(groupId);

			return true;
		} else {
			std::cout << "Big trubs with mismatched lists!" << std::endl;
		}

	}

	std::cout << "get next group is borking: " << mGroupsAvailable.empty() << " " << mGroupsReceived.empty() << std::endl;

	return false;
}

void DeChunker::clearReceived(){
	mReceived.clear();
	mDataChunks.clear();
	mGroupsAvailable.clear();
	mGroupsReceived.clear();
}

}
}