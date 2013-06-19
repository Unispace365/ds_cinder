#include "ds/ui/sprite/multiline_text.h"

using namespace ci;

namespace ds {
namespace ui {

MultilineText::MultilineText(SpriteEngine& engine)
    : inherited(engine)
{
  mMultilineLayout.installOn(*this);
}

MultilineText::~MultilineText()
{
}

float MultilineText::getLeading() const
{
  return mMultilineLayout.mLeading;
}

MultilineText& MultilineText::setLeading(const float v)
{
  if (v == mMultilineLayout.mLeading) return *this;

  mMultilineLayout.mLeading = v;
  mNeedsLayout = true;
  return *this;
}

ds::ui::Alignment MultilineText::getAlignment() const
{
  return mMultilineLayout.mAlignment;
}

MultilineText& MultilineText::setAlignment( const Alignment a )
{
  if (a == mMultilineLayout.mAlignment)
    return *this;

  mMultilineLayout.mAlignment = a;
  mNeedsLayout = true;
  return *this;
}

float MultilineText::getLineHeightWithLeading() const
{
	if (!mFont) return 0.0f;
	return mFont->pointSize() * (1.0 + getLeading() );
}

} // namespace ui
} // namespace ds
