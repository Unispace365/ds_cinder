#include "text.h"
#include <map>
#include "cinder/Vector.h"
#include "cinder/app/App.h"
#include "cinder/Buffer.h"
#include "cinder/DataSource.h"

using namespace ci;

namespace {

std::map<std::string, std::map<float, std::shared_ptr<ci::Font>>> mFontCache;

}

namespace ds {
namespace ui {

Text::Text( SpriteEngine& engine, const std::string &string, const std::string &filename, float fontSize )
    : inherited(engine)
    , mFontSize(fontSize)
    , mTextString(string)
    , mBoxChanged(false)
{
    setTransparent(false);
    mFont = getFont( filename, mFontSize );

    mTextBox.setAlignment(ci::TextBox::LEFT);
    mTextBox.font(*mFont);
    mTextBox.setSize( ci::Vec2i(ci::TextBox::GROW, ci::TextBox::GROW) );
    mTextBox.setText(mTextString);
    mTextBox.setColor( Color( 1.0f, 1.0f, 1.0f ) );

    ci::Vec2i sz = mTextBox.getSize();
    Sprite::setSize(static_cast<float>(sz.x), static_cast<float>(sz.y));

    mBoxChanged = true;
}

Text::~Text()
{

}

void Text::drawLocalClient()
{
    if ( mBoxChanged )
    {
        mTexture = ci::gl::Texture( mTextBox.render() );
        mBoxChanged = false;
    }
    if ( mTexture )
        ci::gl::draw(mTexture);
}

void Text::setSize( float width, float height )
{
    mTextBox.setSize( ci::Vec2i(static_cast<int>(width), static_cast<int>(height)) );
    ci::Vec2i sz = mTextBox.getSize();
    Sprite::setSize(static_cast<float>(sz.x), static_cast<float>(sz.y));
}

void Text::loadText( const std::string &filename )
{
    mFont = getFont( filename, mFontSize );

    if ( !mFont )
        return;

    mTextBox.font(*mFont);

    ci::Vec2i sz = mTextBox.getSize();
    Sprite::setSize(static_cast<float>(sz.x), static_cast<float>(sz.y));
    
    mBoxChanged = true;
}

std::shared_ptr<ci::Font> Text::getFont( const std::string &filename, float fontSize )
{
    auto found = mFontCache.find(filename);
    if ( found != mFontCache.end() )
    {
        auto found2 = found->second.find(fontSize);
        if ( found2 != found->second.end() )
            return found2->second;
    }

    mFontCache[filename][fontSize] = std::move(std::shared_ptr<ci::Font>(new ci::Font));

    ci::DataSourcePathRef src = ci::DataSourcePath::create(filename);

    *(mFontCache[filename][fontSize]) = ci::Font( src, fontSize );
    return mFontCache[filename][fontSize];
}

void Text::setText( const std::string &text )
{
    mTextString = text;
    mTextBox.setText(mTextString);

    mBoxChanged = true;
}

std::string Text::getText() const
{
    return mTextString;
}

void Text::setAlignment( int alignment )
{
    if ( alignment == LEFT )
        mTextBox.setAlignment(ci::TextBox::LEFT);
    else if ( alignment == RIGHT )
        mTextBox.setAlignment(ci::TextBox::RIGHT);
    else if ( alignment == CENTER )
        mTextBox.setAlignment(ci::TextBox::CENTER);
    mBoxChanged = true;
}

void Text::setColor( const ci::Color &color )
{
    mColor = color;
    mBoxChanged = true;
}

void Text::setColor( float r, float g, float b )
{
    mColor = Color(r, g, b);
    mBoxChanged = true;
}

} // namespace ui
} // namespace ds
