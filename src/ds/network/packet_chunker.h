#pragma once
#ifndef CHUNKER_DS_H
#define CHUNKER_DS_H
#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <list>

namespace ds {
namespace net {

struct ChunkHeader {
	size_t mGroupId;
	size_t mSize;
	size_t mId;
	size_t mTotal;
	size_t mChunkSize;
};

/// Chunker splits packets up into byte-sized pieces. HA! Wordplay!
class Chunker {

public:
	Chunker();
	void Chunkify(const char *src, size_t size, size_t groupId, std::vector<std::string> &dst);
	void Chunkify(std::string src, size_t groupId, std::vector<std::string> &dst);

private:
	unsigned mChunkSize;
	std::string mCompressedBuffer;
};

/// DeChunker recombines the pieces into a single unit
class DeChunker {
public:
	DeChunker();
	bool addChunk(const char *chunk, size_t size);
	bool addChunk(std::string &chunk);
	bool getNextGroup(std::string &dst);
	void clearReceived();
	size_t getAvailable() { return mGroupsAvailable.size(); }

private:
	struct DeChunkStats	{
		DeChunkStats(){}
		DeChunkStats(DeChunkStats &&rhs)		{
			mData = std::move(rhs.mData);
			mIdsMissing = rhs.mIdsMissing;
		}

		std::list<size_t> mIdsMissing;
		std::unique_ptr<std::string> mData;
	};

	void addChunkToGroup(DeChunkStats &stats, const char *chunk, size_t size);

	std::map<size_t, DeChunkStats>				mDataChunks;
	std::vector<size_t>							mGroupsAvailable;
	std::vector<size_t>							mGroupsReceived;
	std::list<size_t>							mReceived;
	size_t										mMaxReceivedSize;
	std::vector<std::unique_ptr<std::string>>	mReserveStrings;
};

}
}

#endif//CHUNKER_DS_H
