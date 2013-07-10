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
    virtual ~SpriteShader();
    void setShaders(const std::string &location, const std::string &name);
    void loadShaders();
    bool isValid() const;

    cinder::gl::GlslProg &getShader();

    std::string getLocation() const;
    std::string getName() const;
  private:
    void loadShadersFromFile();
    void loadDefaultFromFile();
    void loadDefaultFromMemory();

    std::string         mDefaultLocation;
    std::string         mDefaultName;
    std::string         mLocation;
    std::string         mName;
    ci::gl::GlslProg    mShader;
};

}
}

#endif//DS_UI_SPRITE_SHADER_H
