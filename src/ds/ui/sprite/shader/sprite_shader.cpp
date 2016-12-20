#include "sprite_shader.h"
#include "cinder/DataSource.h"
#include "ds/debug/logger.h"

#include <Poco/File.h>

namespace {

const std::string DefaultFrag = 
"uniform sampler2D tex0;\n"
"uniform bool useTexture;\n"
"uniform bool preMultiply;\n"
"void main()\n"
"{\n"
"    vec4 color = vec4(1.0, 1.0, 1.0, 1.0);\n"
"    if (useTexture) {\n"
"        color = texture2D( tex0, gl_TexCoord[0].st );\n"
"    }\n"
"    color *= gl_Color;\n"
"    if (preMultiply) {\n"
"        color.r *= color.a;\n"
"        color.g *= color.a;\n"
"        color.b *= color.a;\n"
"    }\n"
"    gl_FragColor = color;\n"
"}\n";

const std::string DefaultVert = 
"void main()\n"
"{\n"
"  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
"  gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;\n"
"  gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;\n"
"  gl_FrontColor = gl_Color;\n"
"}\n";

const ds::BitMask SHADER_LOG = ds::Logger::newModule("shader");

std::map<std::string, ci::gl::GlslProgRef> GlslProgs;

}

namespace ds {
namespace ui {

SpriteShader::SpriteShader(const std::string &defaultLocation, const std::string &defaultName)
	: mDefaultLocation(defaultLocation)
	, mDefaultName(defaultName)
	//, mMemoryVert(nullptr)
	//, mMemoryFrag(nullptr)
{
	mLocation = mDefaultLocation;
	mName = mDefaultName;
}


SpriteShader::SpriteShader(const std::string& vert_memory, const std::string& frag_memory, std::string &shaderName)
	: mMemoryVert(vert_memory)
	, mMemoryFrag(frag_memory)
	, mName(shaderName)
{

}

SpriteShader::~SpriteShader()
{

}


void SpriteShader::setShaders(const std::string& vert_memory, const std::string& frag_memory, std::string &shaderName){
	if(mShader)
		mShader.reset();

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

	if(mShader)
		mShader.reset();

	mLocation = location;
	mName = name;
}

void SpriteShader::loadShaders(){
	loadShadersFromFile();
	if(!mShader)
		loadFromMemory();
	if(!mShader)
		loadDefaultFromFile();
	if(!mShader)
		loadDefaultFromMemory();
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
		auto found = GlslProgs.find(mDefaultName);
		if(found == GlslProgs.end()) {
			mShader = ci::gl::GlslProg::create(ci::loadFile((mDefaultLocation + "/" + mDefaultName + ".vert").c_str()), ci::loadFile((mDefaultLocation + "/" + mDefaultName + ".frag").c_str()));
			GlslProgs[mDefaultName] = mShader;
		} else {
			mShader = found->second;
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
			mShader = ci::gl::GlslProg::create(mMemoryVert.c_str(), mMemoryFrag.c_str());
			GlslProgs[mName] = mShader;
		} else {
			mShader = found->second;
		}
	} catch(std::exception &e) {
		//std::cout << e.what() << std::endl;
		DS_LOG_WARNING_M(std::string("SpriteShader::loadShadersFromFile() on custom file\n") + e.what(), SHADER_LOG);
	}

}

void SpriteShader::loadDefaultFromMemory(){
	try {
		auto found = GlslProgs.find("default_cpp_shader");
		if(found == GlslProgs.end()) {
			mShader = ci::gl::GlslProg::create(DefaultVert.c_str(), DefaultFrag.c_str());
			GlslProgs["default_cpp_shader"] = mShader;
		} else {
			mShader = found->second;
		}
	} catch(std::exception &e) {
		//std::cout << e.what() << std::endl;
		DS_LOG_WARNING_M(std::string("SpriteShader::loadShadersFromFile() on DefaultVert\n") + e.what(), SHADER_LOG);
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
