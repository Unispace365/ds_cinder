#pragma once
#ifndef DS_UI_IMAGESOURCE_IMAGEARC_H_
#define DS_UI_IMAGESOURCE_IMAGEARC_H_

#include <string>
#include <vector>
#include "ds/ui/image_source/image_source.h"

namespace ds {
class ImageRegistry;

namespace ui {

/**
 * \class ds::ui::ImageArc
 * \brief Generate an image with an arc.
 */
class ImageArc : public ImageSource
{
public:
  /**
   * \param filename is the filename (and path) for the resource.
	 * If filename begins with "resource:" then this is a compiled-in resource.
   */
	ImageArc(const int width, const int height, const std::string& filename);

	virtual ImageGenerator*		newGenerator(SpriteEngine&) const;

	void											addInput(const float);

private:
	const int									mWidth,
														mHeight;
	const std::string					mFilename;
	std::vector<float>				mFloatInput;

	// Engine initialization
public:
	// Generators must be registered with the system at startup (i.e. in App constructor)
	static void								install(ds::ImageRegistry&);
};

} // namespace ui
} // namespace ds

#endif // DS_UI_IMAGESOURCE_IMAGEARC_H_
