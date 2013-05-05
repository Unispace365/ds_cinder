#pragma once
#ifndef DS_APP_IMAGESOURCEREGISTRY_H_
#define DS_APP_IMAGESOURCEREGISTRY_H_

#include <functional>
#include <vector>

namespace ds {
class BlobReader;

namespace ui {
class ImageSource;
class SpriteEngine;
} // namespace ui

/**
 * \class ds::ImageSourceRegistry
 * Global registry of all objects that can generate images.
 */
class ImageSourceRegistry {
  public:
    ImageSourceRegistry();

    // Add a new blob handler.  I answer with the unique key assigned the handler.
    char									add(const std::function<ds::ui::ImageSource*(ds::ui::SpriteEngine&)>& factoryFn);
		ds::ui::ImageSource*	make(const char, ds::ui::SpriteEngine&);

  private:
    std::vector<std::function<ds::ui::ImageSource*(ds::ui::SpriteEngine&)>>
                      mFactory;
};

} // namespace ds

#endif // DS_APP_IMAGESOURCEREGISTRY_H_