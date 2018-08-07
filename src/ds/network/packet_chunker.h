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
	unsigned mGroupId;
	unsigned mSize;
	unsigned mId;
	unsigned mTotal;
	unsigned mChunkSize;
};

/// Chunker splits packets up into byte-sized pieces. HA! Wordplay!
class Chunker {

public:
	Chunker();
	void Chunkify(const char *src, unsigned size, unsigned groupId, std::vector<std::string> &dst);
	void Chunkify(std::string src, unsigned groupId, std::vector<std::string> &dst);

private:
	unsigned mChunkSize;
	std::string mCompressedBuffer;
};

/// DeChunker recombines the pieces into a single unit
class DeChunker {
public:
	DeChunker();
	bool addChunk(const char *chunk, unsigned size);
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

		std::list<unsigned> mIdsMissing;
		std::unique_ptr<std::string> mData;
	};

	void addChunkToGroup(DeChunkStats &stats, const char *chunk, unsigned size);

	std::map<unsigned, DeChunkStats>				mDataChunks;
	std::vector<unsigned>							mGroupsAvailable;
	std::vector<unsigned>							mGroupsReceived;
	std::list<unsigned>							mReceived;
	unsigned										mMaxReceivedSize;
	std::vector<std::unique_ptr<std::string>>	mReserveStrings;
};

}
}

#endif//CHUNKER_DS_H
