#pragma once
#ifndef DS_UI_IMAGESOURCE_IMAGEGLSL_H_
#define DS_UI_IMAGESOURCE_IMAGEGLSL_H_

#include <string>
#include <vector>
#include "ds/gl/uniform.h"
#include "ds/ui/image_source/image_source.h"

namespace ds {
class ImageRegistry;

namespace ui {

/**
 * \class ds::ui::ImageGlsl
 * \brief Generate an image with a GLSL shader.
 */
class ImageGlsl : public ImageSource {
public:
	/**
	 * \param width The width of the image
	 * \param height The height of the image
	 * \param filestem is the file stem for both the vertex and fragment shaders.
	 * It's assumed to be located in the standard app data/shaders path, and the
	 * files have the .vert and .frag extensions.
	 */
	ImageGlsl(const int width, const int height, const std::string& filestem);

	/**
	 * \param width The width of the image
	 * \param height The height of the image
	 * \param vertexFilename The absolute paths to the vertex shader.
	 * \param fragmentFilename The absolute paths to the fragment shader.
	 */
	ImageGlsl(const int width, const int height, const std::string& vertexFilename, const std::string& fragmentFilename);

	virtual ImageGenerator*	newGenerator(SpriteEngine&) const;
	ds::gl::Uniform&		getUniform();
	
private:
	const int				mWidth,
							mHeight;
	const std::string		mVertexFilename,
							mFragmentFilename;
	ds::gl::Uniform			mUniform;

	// Engine initialization
public:
	// Generators must be registered with the system at startup (i.e. in App constructor)
	static void				install(ds::ImageRegistry&);
};

} // namespace ui
} // namespace ds

#endif // DS_UI_IMAGESOURCE_IMAGEGLSL_H_
