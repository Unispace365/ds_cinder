#ifndef _LAYOUT_BUILDER_APP_APPEVENTS_H_
#define _LAYOUT_BUILDER_APP_APPEVENTS_H_

#include <ds/app/event.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/text.h>

namespace layout_builder {

class IdleStartedEvent : public ds::RegisteredEvent < IdleStartedEvent > {
public:
	IdleStartedEvent(){};
};

class IdleEndedEvent : public ds::RegisteredEvent < IdleEndedEvent > {
public:
	IdleEndedEvent(){};

};

class LoadLayoutRequest : public ds::RegisteredEvent<LoadLayoutRequest>{
public:
	LoadLayoutRequest(const std::string& layoutLocation) : mLocation(layoutLocation) {}
	const std::string mLocation;
};

class AppRestartRequest : public ds::RegisteredEvent<AppRestartRequest>{
public:
	AppRestartRequest(){};
};

class RefreshLayoutRequest : public ds::RegisteredEvent<RefreshLayoutRequest>{
public:
	RefreshLayoutRequest(){};
};

class SaveLayoutRequest : public ds::RegisteredEvent<SaveLayoutRequest>{
public:
	SaveLayoutRequest(){};
};

class LayoutLayoutRequest : public ds::RegisteredEvent<LayoutLayoutRequest>{
public:
	LayoutLayoutRequest(){};
};

class AnimateLayoutRequest : public ds::RegisteredEvent<AnimateLayoutRequest>{
public:
	AnimateLayoutRequest(){};
};

class InspectTreeRequest : public ds::RegisteredEvent<InspectTreeRequest>{
public:
	// ownership of the tree doesn't change
	InspectTreeRequest(ds::ui::Sprite* sp) :mTree(sp){}
	ds::ui::Sprite*		mTree;
};

class InspectSpriteRequest : public ds::RegisteredEvent<InspectSpriteRequest>{
public:
	// ownership of the tree doesn't change
	InspectSpriteRequest(ds::ui::Sprite* sp) :mSprid(sp){}
	ds::ui::Sprite*		mSprid;
};

class InputFieldSetRequest : public ds::RegisteredEvent<InputFieldSetRequest>{
public:
	// sets the text sprite to capute text input
	// send nullptr to clear the input field
	InputFieldSetRequest(ds::ui::Text* inputField) : mInputField(inputField){}
	ds::ui::Text*		mInputField;
};

class InputFieldCleared : public ds::RegisteredEvent<InputFieldCleared>{
public:
	InputFieldCleared(){}
};

class InputFieldTextInput : public ds::RegisteredEvent<InputFieldTextInput>{
public:
	InputFieldTextInput(ds::ui::Text* inputField, const std::wstring fullText) : mInputField(inputField), mFullText(fullText){}
	ds::ui::Text*		mInputField;
	const std::wstring	mFullText;

};

class CreateSpriteRequest : public ds::RegisteredEvent<CreateSpriteRequest>{
public:
	CreateSpriteRequest(const std::string& typeName) : mTypeName(typeName){}
	const std::string	mTypeName;
};

class DeleteSpriteRequest : public ds::RegisteredEvent<DeleteSpriteRequest>{
public:
	DeleteSpriteRequest(ds::ui::Sprite* spriteToDelete) : mSpriteToDelete(spriteToDelete){}
	ds::ui::Sprite*		mSpriteToDelete;
};

class MouseMoveEvent : public ds::RegisteredEvent<MouseMoveEvent>{
public:
	MouseMoveEvent( const ci::Vec3f mousePoint ) : mMousePoint(mousePoint) {}
	ci::Vec3f			mMousePoint;
};

class ShowSpriteHighlightEvent : public ds::RegisteredEvent<ShowSpriteHighlightEvent>{
public:
	ShowSpriteHighlightEvent( ds::ui::Sprite* spriteToHighlight ) : mSpriteToHighlight( spriteToHighlight ){}
	ds::ui::Sprite*		mSpriteToHighlight;
};


} // !namespace layout_builder

#endif // !_LAYOUT_BUILDER_APP_APPEVENTS_H_