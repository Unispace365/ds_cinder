#pragma once
#ifndef DS_UI_VIEW_IMAGEVIEW_H_
#define DS_UI_VIEW_IMAGEVIEW_H_

#include "ds/ui/view/view.h"

namespace ds {

/**
 * \class ds::ImageView
 */
class ImageView : public View {
public:
	ImageView(Engine&);

private:
	typedef	View		inherited;
};

} // namespace ds

#endif // DS_UI_VIEW_IMAGEVIEW_H_