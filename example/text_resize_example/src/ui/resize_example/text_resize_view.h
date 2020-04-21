#pragma once

#include <ds/ui/layout/smart_layout.h>

namespace text_resize {

/**
* \class text_resize::TextResizeView
*			A view that shows a single story. Disappears when idle starts, and reappears on user action
*/
class TextResizeView : public ds::ui::SmartLayout  {
public:
	TextResizeView(ds::ui::SpriteEngine& eng);
protected:
	void onUpdateServer(const ds::UpdateParams& p) override;
private:
	bool mIsDirty = true;
	bool mUserMoved = false;
	ds::ui::Text*	mTextSprite;
	ds::ui::Sprite* mTextBoxBack;
	ds::ui::Sprite* mLimitBack;
	ds::ui::Sprite* mResizeHandle;
};

} // namespace text_resize

