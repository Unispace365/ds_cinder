#include "stdafx.h"

#include "ds/app/blob_reader.h"

namespace ds {

BlobReader::BlobReader(DataBuffer& db, ui::SpriteEngine& se)
  : mDataBuffer(db)
  , mSpriteEngine(se) {}

} // namespace ds