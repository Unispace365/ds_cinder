#pragma once
#ifndef DS_APP_BLOBREADER_H_
#define DS_APP_BLOBREADER_H_

namespace ds {
class DataBuffer;

namespace ui {
	class SpriteEngine;
} // namespace ui

/**
 * \class BlobReader
 * A collection class for the object used in the blob registry.
 */
class BlobReader {
  public:
	BlobReader(DataBuffer&, ui::SpriteEngine&);
	~BlobReader() = default;

	BlobReader()							 = delete;
	BlobReader(const BlobReader&)			 = delete;
	BlobReader(BlobReader&&)				 = delete;
	BlobReader& operator=(const BlobReader&) = delete;
	BlobReader& operator=(BlobReader&&)		 = delete;
	
	DataBuffer&		  mDataBuffer;
	ui::SpriteEngine& mSpriteEngine;
};

} // namespace ds

#endif // DS_APP_BLOBREADER_H_
