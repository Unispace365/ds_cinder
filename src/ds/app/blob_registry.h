#pragma once
#ifndef DS_APP_BLOBREGISTRY_H_
#define DS_APP_BLOBREGISTRY_H_

#include <functional>
#include <vector>

namespace ds {
class BlobReader;

/**
 * \class ds::BlobRegistry
 * Global handlers on flattenable data clients.  Someone who wants to
 * participate in reading and writing to a byte stream will install
 * a handler on me.
 */
class BlobRegistry {
  public:
    BlobRegistry();

    // Add a new blob handler.  I answer with the unique key assigned the handler.
    char              add(const std::function<void(BlobReader&)>& reader);

  private:
    friend class EngineClient;
    friend class EngineServer;
    friend class EngineReceiver;
    std::vector<std::function<void(BlobReader&)>>
                      mReader;
};

} // namespace ds

#endif // DS_APP_BLOBREGISTRY_H_