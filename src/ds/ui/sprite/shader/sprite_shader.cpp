#include "stdafx.h"

#include "sprite_shader.h"
#include "cinder/DataSource.h"
#include "ds/debug/logger.h"
#include <ds/util/file_meta_data.h>

#include <Poco/File.h>

namespace {

const std::string DefaultFrag = 
//"#version 150\n"
"uniform sampler2D  tex0;\n"
"uniform bool       useTexture;\n"
"uniform bool       preMultiply;\n"
"in vec2            TexCoord0;\n"
"in vec4            Color;\n"
"out vec4           oColor;\n"
"void main()\n"
"{\n"
"    oColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
"    if (useTexture) {\n"
"        oColor = texture2D( tex0, TexCoord0 );\n"
"    }\n"
"    oColor *= Color;\n"
"    if (preMultiply) {\n"
"        oColor.r *= oColor.a;\n"
"        oColor.g *= oColor.a;\n"
"        oColor.b *= oColor.a;\n"
"    }\n"
"}\n";

const std::string DefaultVert = 
"#version 150\n"
"uniform mat4       ciModelMatrix;\n"
"uniform mat4       ciModelViewProjection;\n"
"uniform vec4       uClipPlane0;\n"
"uniform vec4       uClipPlane1;\n"
"uniform vec4       uClipPlane2;\n"
"uniform vec4       uClipPlane3;\n"
"in vec4            ciPosition;\n"
"in vec2            ciTexCoord0;\n"
"in vec4            ciColor;\n"
"out vec2           TexCoord0;\n"
"out vec4           Color;\n"
"void main()\n"
"{\n"
"    gl_Position = ciModelViewProjection * ciPosition;\n"
"    TexCoord0 = ciTexCoord0;\n"
"    Color = ciColor;\n"
"    gl_ClipDistance[0] = dot(ciModelMatrix * ciPosition, uClipPlane0);\n"
"    gl_ClipDistance[1] = dot(ciModelMatrix * ciPosition, uClipPlane1);\n"
"    gl_ClipDistance[2] = dot(ciModelMatrix * ciPosition, uClipPlane2);\n"
"    gl_ClipDistance[3] = dot(ciModelMatrix * ciPosition, uClipPlane3);\n"
"}\n";

const std::string NoImageFrag =
//"#version 150\n"
"uniform int		tex0;\n"
"uniform bool       useTexture;\n"
"uniform bool       preMultiply;\n"
"in vec4            Color;\n"
"out vec4           oColor;\n"
"void main()\n"
"{\n"
"    oColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
"    if (useTexture) {\n"
"    }\n"
"    oColor *= Color;\n"
"    if (preMultiply) {\n"
"        oColor.r *= oColor.a;\n"
"        oColor.g *= oColor.a;\n"
"        oColor.b *= oColor.a;\n"
"    }\n"
"}\n";

const std::string NoImageVert =
"#version 150\n"
"uniform mat4       ciModelMatrix;\n"
"uniform mat4       ciModelViewProjection;\n"
"uniform vec4       uClipPlane0;\n"
"uniform vec4       uClipPlane1;\n"
"uniform vec4       uClipPlane2;\n"
"uniform vec4       uClipPlane3;\n"
"in vec4            ciPosition;\n"
"in vec4            ciColor;\n"
"out vec4           Color;\n"
"void main()\n"
"{\n"
"    gl_Position = ciModelViewProjection * ciPosition;\n"
"    Color = ciColor;\n"
"    gl_ClipDistance[0] = dot(ciModelMatrix * ciPosition, uClipPlane0);\n"
"    gl_ClipDistance[1] = dot(ciModelMatrix * ciPosition, uClipPlane1);\n"
"    gl_ClipDistance[2] = dot(ciModelMatrix * ciPosition, uClipPlane2);\n"
"    gl_ClipDistance[3] = dot(ciModelMatrix * ciPosition, uClipPlane3);\n"
"}\n";

const ds::BitMask SHADER_LOG = ds::Logger::newModule("shader");

std::unordered_map<std::string, ci::gl::GlslProgRef> GlslProgs;

}

namespace ds {
namespace ui {

SpriteShader::SpriteShader(const std::string &defaultLocation, const std::string &defaultName)
	: mDefaultLocation(defaultLocation)
	, mDefaultName(defaultName)
	, mShader(nullptr)
{
	mLocation = mDefaultLocation;
	mName = mDefaultName;
}


SpriteShader::SpriteShader(const std::string& vert_memory, const std::string& frag_memory, const std::string &shaderName)
	: mMemoryVert(vert_memory)
	, mMemoryFrag(frag_memory)
	, mName(shaderName)
	, mShader(nullptr)
{
}

void SpriteShader::setShaders(const std::string& vert_memory, const std::string& frag_memory, const std::string &shaderName){
	if(mShader) {
		mShader.reset();
	}

	mMemoryVert = vert_memory;
	mMemoryFrag = frag_memory;
	mName = shaderName;
}

void SpriteShader::setShaders(const std::string &location, const std::string &name){
	if(name.empty()) {
		DS_LOG_WARNING_M("SpriteShader::setShaders() on empty name, did you intend that?", SHADER_LOG);
		return;
	}

	if(mLocation == location && mName == name) {
		return;
	}

	if(mShader){
		mShader.reset();
	}

	mLocation = location;
	mName = name;
}

void SpriteShader::setToDefaultShader(){
	if(mShader){
		mShader.reset();
	}

	mName = "base";
	loadDefaultFromMemory();
}

void SpriteShader::setToNoImageShader() {
	if(mShader) {
		mShader.reset();
	}

	mName = "noImage";
	loadNoImageFromMemory();
}

void SpriteShader::loadShaders() {
	if(mShader) return;
	if(!mShader) loadShadersFromFile();
	if(!mShader) loadFromMemory();
	if(!mShader) loadDefaultFromFile();
	if(!mShader) loadDefaultFromMemory();
}

bool SpriteShader::isValid() const {
	return (mShader != nullptr);
}

ci::gl::GlslProgRef SpriteShader::getShader(){
	return mShader;
}

void SpriteShader::loadShadersFromFile(){
	if(mName.empty())
		return;

	try {
		auto found = GlslProgs.find(mName);
		if(found == GlslProgs.end()) {
			std::string vertLocation = (mLocation + "/" + mName + ".vert");
			std::string fragLocation = (mLocation + "/" + mName + ".frag");
			Poco::File vertFile = Poco::File(vertLocation);
			Poco::File fragFile = Poco::File(fragLocation);
			bool exist = false;
			try{
				if(vertFile.exists() && fragFile.exists()){
					exist = true;
				} 
			} catch(std::exception&){
				// swallow these, cause shaders could be loaded otherwise
			}
			if(exist){
				mShader = ci::gl::GlslProg::create(ci::loadFile(vertLocation.c_str()), ci::loadFile(fragLocation.c_str()));
				GlslProgs[mName] = mShader;
			}
		} else {
			mShader = found->second;
		}
	} catch(std::exception &e) {
		//std::cout << e.what() << std::endl;
		if(mName.empty()) {
			DS_LOG_WARNING_M("SpriteShader::loadShadersFromFile() on non empty name, did you intend that?", SHADER_LOG);
		} else {
			DS_LOG_WARNING_M("SpriteShader::loadShadersFromFile() on " + mName + "\n" + e.what(), SHADER_LOG);
		}
	}
}

void SpriteShader::loadDefaultFromFile(){
	if(mDefaultName.empty())
		return;

	try {
		std::string vertLocation = (mDefaultLocation + "/" + mDefaultName + ".vert");
		std::string fragLocation = (mDefaultLocation + "/" + mDefaultName + ".frag");
		if(!ds::safeFileExistsCheck(vertLocation) || !ds::safeFileExistsCheck(fragLocation)){
			// swallow these errors?
		} else {
			auto found = GlslProgs.find(mDefaultName);
			if(found == GlslProgs.end()) {
				mShader = ci::gl::GlslProg::create(ci::loadFile((mDefaultLocation + "/" + mDefaultName + ".vert").c_str()), ci::loadFile((mDefaultLocation + "/" + mDefaultName + ".frag").c_str()));
				GlslProgs[mDefaultName] = mShader;
			} else {
				mShader = found->second;
			}
		}

	} catch(std::exception &e) {
		//    std::cout << e.what() << std::endl;
		DS_LOG_WARNING_M("SpriteShader::loadShadersFromFile() on " + mDefaultName + "\n" + e.what(), SHADER_LOG);
	}
}

void SpriteShader::loadFromMemory(){
	try {
		auto found = GlslProgs.find(mName);
		if(found == GlslProgs.end()) {
			if(mMemoryVert.empty() || mMemoryFrag.empty()){
				// swallow these errors?
			} else {
				mShader = ci::gl::GlslProg::create(mMemoryVert.c_str(), mMemoryFrag.c_str());
				GlslProgs[mName] = mShader;
			}
		} else {
			mShader = found->second;
		}
	} catch(std::exception &e) {
		//std::cout << e.what() << std::endl;
		DS_LOG_WARNING_M("SpriteShader::loadShadersFromMemory() " << mName << " " << e.what(), SHADER_LOG);
	}

}

void SpriteShader::loadDefaultFromMemory(){
	try {
		auto found = GlslProgs.find("base");
		if(found == GlslProgs.end()) {
			mShader = ci::gl::GlslProg::create(DefaultVert.c_str(), DefaultFrag.c_str());
			GlslProgs["base"] = mShader;
		} else {
			mShader = found->second;
		}
	} catch(std::exception &e) {
		//std::cout << e.what() << std::endl;
		DS_LOG_WARNING_M(std::string("SpriteShader::loadDefaultFromMemory() on DefaultVert\n") + e.what(), SHADER_LOG);
	}
}

void SpriteShader::loadNoImageFromMemory() {
	try {
		auto found = GlslProgs.find("noImage");
		if(found == GlslProgs.end()) {
			mShader = ci::gl::GlslProg::create(NoImageVert.c_str(), NoImageFrag.c_str());
			GlslProgs["noImage"] = mShader;
		} else {
			mShader = found->second;
		}
	} catch(std::exception &e) {
		//std::cout << e.what() << std::endl;
		DS_LOG_WARNING_M(std::string("SpriteShader::loadDefaultFromMemory() on NoImage\n") + e.what(), SHADER_LOG);
	}
}

std::string SpriteShader::getLocation() const{
	return mLocation;
}

std::string SpriteShader::getName() const{
	return mName;
}


void SpriteShader::clearShaderCache() {
	GlslProgs.clear();
}

}
}
