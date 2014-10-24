#pragma once
#ifndef _INTERFACEXMLIMPORTEREXAMPLEAPP_VIEW_SAMPLE_VIEW_H_
#define _INTERFACEXMLIMPORTEREXAMPLEAPP_VIEW_SAMPLE_VIEW_H_

#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/text.h>
#include <ds/ui/sprite/multiline_text.h>

namespace importer_example {
class Globals;

/**
* \class importer_example::SampleView
* \brief An example of how to load an xml layout into a view
*/
class SampleView : public ds::ui::Sprite {
public:
	SampleView(Globals&);

private:
	typedef ds::ui::Sprite					inherited;
	void									dispatchOpenEvent();

	Globals&								mGlobals;

	ds::ui::Text*							mTitle;
	ds::ui::MultilineText*					mBody;
	ds::ui::Image*							mSampleImage;

	std::map<std::string, ds::ui::Sprite*>	mSpriteMap;

};

} // namespace importer_example

#endif // _INTERFACEXMLIMPORTEREXAMPLEAPP_VIEW_SAMPLE_VIEW_H_
