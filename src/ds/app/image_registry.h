#pragma once
#ifndef DS_APP_IMAGEREGISTRY_H_
#define DS_APP_IMAGEREGISTRY_H_

#include <functional>
#include <vector>

namespace ds {
class BlobReader;

namespace ui {
class ImageGenerator;
class SpriteEngine;
} // namespace ui

/**
 * \class ds::ImageRegistry
 * Global registry of all objects that can generate images.
 */
class ImageRegistry {
  public:
    ImageRegistry();

    // Add a new blob handler.  I answer with the unique key assigned the handler.
    char										addGenerator(const std::function<ds::ui::ImageGenerator*(ds::ui::SpriteEngine&)>& factoryFn);
		ds::ui::ImageGenerator*	makeGenerator(const char, ds::ui::SpriteEngine&);

  private:
    std::vector<std::function<ds::ui::ImageGenerator*(ds::ui::SpriteEngine&)>>
                      mFactory;
};

} // namespace ds

#endif // DS_APP_IMAGEREGISTRY_H_