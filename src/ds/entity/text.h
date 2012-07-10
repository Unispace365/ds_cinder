#pragma once
#ifndef DS_TEXT_H
#define DS_TEXT_H
#include "entity.h"
#include <string>
#include "cinder/gl/Texture.h"
#include "cinder/Text.h"
#include "cinder/Font.h"

namespace ds
{

class Text: public Entity
{
    public:
        enum Alignment
        {
            LEFT,
            RIGHT,
            CENTER
        };

        Text( const std::string &string, const std::string &filename, float fontSize );
        ~Text();
        void                      setSize( float width, float height );
        void                      setAlignmnet( int alignment );
        void                      drawLocal();
        void                      loadText( const std::string &filename );
        void                      setText( const std::string &text );
        std::string               getText() const;

        virtual void              setColor( const ci::Color &color );
        virtual void              setColor( float r, float g, float b );
    private:
        std::shared_ptr<ci::Font> getFont( const std::string &filename, float fontSize );
        std::shared_ptr<ci::Font> mFont;
        ci::TextBox               mTextBox;
        ci::gl::Texture           mTexture;

        bool                      mBoxChanged;
        float                     mFontSize;
        std::string               mTextString;
};

}

#endif//DS_TEXT_H
