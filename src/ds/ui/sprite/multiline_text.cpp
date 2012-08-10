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

} // namespace ui
} // namespace ds
