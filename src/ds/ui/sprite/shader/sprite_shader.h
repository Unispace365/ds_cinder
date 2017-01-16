#pragma once
#ifndef DS_UI_SPRITE_SHADER_H
#define DS_UI_SPRITE_SHADER_H
#include "cinder/gl/GlslProg.h"
#include <string>

namespace ds {
namespace ui {

class SpriteShader
{
public:
	SpriteShader(const std::string &defaultLocation, const std::string &defaultName);
	SpriteShader(const std::string& vert_memory, const std::string& frag_memory, std::string &shaderName);

	virtual ~SpriteShader();
	void setShaders(const std::string &location, const std::string &name);
	void setShaders(const std::string &vert_memory, const std::string &frag_memory, std::string &shaderName);
	void loadShaders();
	bool isValid() const;

	/**
	 * Clears the cache of loaded GLSL shaders, so that the next time
	 * a sprite sets a shader, it will reload that shader from the
	 * source file.  This is particularly useful during development if
	 * you want to test changes to a shader without restarting the app.
	 */
	static void clearShaderCache();

	ci::gl::GlslProgRef getShader();

	std::string getLocation() const;
	std::string getName() const;
private:
	void loadShadersFromFile();
	void loadFromMemory();

	void loadDefaultFromFile();
	void loadDefaultFromMemory();

	std::string			mDefaultLocation;
	std::string			mDefaultName;
	std::string			mLocation;
	std::string			mName;
	std::string			mMemoryVert;
	std::string			mMemoryFrag;

	ci::gl::GlslProgRef	mShader;
};

}
}

#endif//DS_UI_SPRITE_SHADER_H
