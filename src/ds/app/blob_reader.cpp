#include "stdafx.h"

#include "ds/app/blob_reader.h"

#include "ds/debug/debug_defines.h"
#include <assert.h>
#include <iostream>

namespace ds {

BlobReader::BlobReader(DataBuffer& db, ui::SpriteEngine& se)
  : mDataBuffer(db)
  , mSpriteEngine(se) {}

} // namespace ds