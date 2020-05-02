#pragma once

#include <ds/ui/layout/smart_layout.h>

namespace text_resize {

	class TextResizeView;

	/**
	* \class text_resize::TextResizeController
	*			
	*/
	class TextResizeController : public ds::ui::SmartLayout {
	public:
		TextResizeController(ds::ui::SpriteEngine& eng);
		TextResizeView*	mResizeView = nullptr;
		
	};

} // namespace text_resize


