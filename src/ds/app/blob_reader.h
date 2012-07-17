#pragma once
#ifndef DS_APP_BLOBREADER_H_
#define DS_APP_BLOBREADER_H_

namespace ds {
class DataBuffer;

namespace ui {
class SpriteEngine;
} // namespace ui

/**
 * \class ds::ui::BlobReader
 * A collection class for the object used in the blob registry.
 */
class BlobReader {
  public:
    BlobReader(DataBuffer&, ui::SpriteEngine&);

    DataBuffer&         mDataBuffer;
    ui::SpriteEngine&   mSpriteEngine;

  private:
    BlobReader();
    BlobReader(const BlobReader&);
};

} // namespace ds

#endif // DS_APP_BLOBREADER_H_