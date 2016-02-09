#include "sprite_inspector.h"

#include <Poco/LocalDateTime.h>

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/app/event_notifier.h>
#include <ds/debug/logger.h>
#include <ds/util/string_util.h>
#include <ds/ui/sprite/multiline_text.h>
#include <ds/ui/sprite/gradient_sprite.h>
#include <ds/ui/sprite/circle.h>
#include <ds/ui/sprite/border.h>
#include <ds/ui/sprite/circle_border.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/button/image_button.h>

#include "app/app_defs.h"
#include "app/globals.h"
#include "events/app_events.h"
#include <typeinfo>
#include <ds/ui/interface_xml/interface_xml_importer.h>
#include "ui/sprite_property_item.h"

namespace layout_builder {

SpriteInspector::SpriteInspector(Globals& g)
	: ds::ui::Sprite(g.mEngine)
	, mGlobals(g)
	, mEventClient(g.mEngine.getNotifier(), [this](const ds::Event *m){ if(m) this->onAppEvent(*m); })
	, mLayout(nullptr)
	, mLinkedSprite(nullptr)
	, mCurrentInputTreeItem(nullptr)
{

	setPosition(800.0f, 800.0f);
	enable(true);
	enableMultiTouch(ds::ui::MULTITOUCH_CAN_POSITION);

}

void SpriteInspector::onAppEvent(const ds::Event& in_e){
	if(in_e.mWhat == InspectSpriteRequest::WHAT()){
		const InspectSpriteRequest& e((const InspectSpriteRequest&)in_e);
		inspectSprite(e.mSprid);
	} else if(in_e.mWhat == RefreshLayoutRequest::WHAT() || in_e.mWhat == LoadLayoutRequest::WHAT()){
		clearProperties();
	} else if(in_e.mWhat == InputFieldTextInput::WHAT()){
		if(mCurrentInputTreeItem && mLinkedSprite){
			const InputFieldTextInput& e((const InputFieldTextInput&)in_e);
			std::string valueText = ds::utf8_from_wstr(e.mFullText);
			mCurrentInputTreeItem->setValueText(e.mFullText);
			ds::ui::XmlImporter::setSpriteProperty(*mLinkedSprite, mCurrentInputTreeItem->getPropertyName(), valueText, "");
			mEngine.getNotifier().notify(LayoutLayoutRequest());
			layout();
		}
	} else if(in_e.mWhat == InputFieldCleared::WHAT()){
		setInputField(nullptr);
	}

}

void SpriteInspector::clearProperties(){

	setInputField(nullptr);
	if(mLayout){
		mLayout->release();
		mLayout = nullptr;
	}

	mSpriteProperties.clear();
}

void SpriteInspector::inspectSprite(ds::ui::Sprite* sp) {
	clearProperties();

	if(!sp) return;

	mLinkedSprite = sp;

	// todo: make this an xml
	mLayout = new ds::ui::LayoutSprite(mEngine);
	mLayout->setSpriteName(L"This layout");
	mLayout->setLayoutType(ds::ui::LayoutSprite::kLayoutVFlow);
	//mLayout->setSpacing(5.0f);
	mLayout->setShrinkToChildren(ds::ui::LayoutSprite::kShrinkBoth);
	mLayout->setTransparent(false);
	mLayout->setColor(ci::Color(0.2f, 0.2f, 0.2f));
	addChildPtr(mLayout);

	addSpriteProperty(L"name", sp->getSpriteName());
	addSpriteProperty(L"type", ds::wstr_from_utf8(ds::ui::XmlImporter::getSpriteTypeForSprite(sp)));
	addSpriteProperty(L"size", sp->getSize());
	addSpriteProperty(L"color", sp->getColor());
	addSpriteProperty(L"opacity", sp->getOpacity());
	addSpriteProperty(L"center", sp->getCenter());
	addSpriteProperty(L"position", sp->getPosition());
	addSpriteProperty(L"rotation", sp->getRotation());
	addSpriteProperty(L"scale", sp->getScale());
	addSpriteProperty(L"clipping", sp->getClipping());
	addSpritePropertyBlend(L"blend_mode", sp->getBlendMode());
	addSpriteProperty(L"enable", sp->isEnabled());
	addSpriteProperty(L"multitouch", sp->getMultiTouchConstraints());
	addSpriteProperty(L"transparent", sp->getTransparent());
	addSpriteProperty(L"animate_on", ds::wstr_from_utf8(sp->getAnimateOnScript()));
	addSpriteProperty(L"t_pad", sp->mLayoutTPad);
	addSpriteProperty(L"l_pad", sp->mLayoutLPad);
	addSpriteProperty(L"b_pad", sp->mLayoutBPad);
	addSpriteProperty(L"r_pad", sp->mLayoutRPad);
	addSpriteProperty(L"layout_fudge", sp->mLayoutFudge);
	addSpriteProperty(L"layout_size", sp->mLayoutSize);
	addSpritePropertyLayoutSizeMode(L"layout_size_mode", sp->mLayoutUserType);
	addSpritePropertyLayoutVAlign(L"layout_v_align", sp->mLayoutVAlign);
	addSpritePropertyLayoutHAlign(L"layout_h_align", sp->mLayoutHAlign);

	ds::ui::LayoutSprite* ls = dynamic_cast<ds::ui::LayoutSprite*>(mLinkedSprite);
	if(ls){
		addSpritePropertyLayoutType(L"layout_type", ls->getLayoutType());
		addSpriteProperty(L"layout_spacing", ls->getSpacing());
		addSpritePropertyLayoutShrink(L"shrink_to_children", ls->getShrinkToChildren());
		addSpritePropertyLayoutVAlign(L"overall_alignment", ls->getOverallAlignment());
	}

	ds::ui::Text* texty = dynamic_cast<ds::ui::Text*>(mLinkedSprite);
	if(texty){
		addSpriteProperty(L"text", texty->getText());
		addSpriteProperty(L"font", ds::wstr_from_utf8(texty->getConfigName()));
	//	addSpriteProperty(L"font_size", texty->getFontSize());
	}

	ds::ui::MultilineText* multitexty = dynamic_cast<ds::ui::MultilineText*>(mLinkedSprite);
	if(multitexty){
		addSpriteProperty(L"resize_limit", ci::Vec2f(multitexty->getResizeLimitWidth(), multitexty->getResizeLimitHeight()));
		addSpritePropertyLayoutHAlign(L"text_align", multitexty->getAlignment());
	}

	ds::ui::Gradient* grad = dynamic_cast<ds::ui::Gradient*>(mLinkedSprite);
	if(grad){
		addSpriteProperty(L"gradientColors", ds::wstr_from_utf8(ds::ui::XmlImporter::getGradientColorsAsString(grad)));
	}

	ds::ui::Circle* circle = dynamic_cast<ds::ui::Circle*>(mLinkedSprite);
	if(circle){
		addSpriteProperty(L"filled", circle->getFilled());
		addSpriteProperty(L"radius", circle->getRadius());
	}

	ds::ui::Border* border = dynamic_cast<ds::ui::Border*>(mLinkedSprite);
	if(border){
		addSpriteProperty(L"border_width", border->getBorderWidth());
	}

	ds::ui::Image* img = dynamic_cast<ds::ui::Image*>(mLinkedSprite);
	if(img){
		addSpriteProperty(L"filename", ds::wstr_from_utf8(ds::Environment::contract(img->getImageFilename())));
		addSpriteProperty(L"circle_crop", img->getCircleCrop());
	}

	ds::ui::ImageButton* imgB = dynamic_cast<ds::ui::ImageButton*>(mLinkedSprite);
	if(imgB){
		addSpriteProperty(L"up_image", ds::wstr_from_utf8(ds::Environment::contract(imgB->getNormalImage().getImageFilename())));
		addSpriteProperty(L"down_image", ds::wstr_from_utf8(ds::Environment::contract(imgB->getHighImage().getImageFilename())));
		addSpriteProperty(L"btn_touch_padding", imgB->getPad());
	}

	layout();
	animateOn();
}


void SpriteInspector::addSpriteProperty(const std::wstring& propertyName, const std::wstring& propertyValue){
	doAddSpriteProperty(propertyName, propertyValue);
}

void SpriteInspector::addSpriteProperty(const std::wstring& propertyName, const bool propertyValue){
	if(propertyValue){
		doAddSpriteProperty(propertyName, L"true");
	} else {
		doAddSpriteProperty(propertyName, L"false");
	}
}

void SpriteInspector::addSpriteProperty(const std::wstring& propertyName, const ds::BitMask& propertyValue){
	doAddSpriteProperty(propertyName, ds::wstr_from_utf8(ds::ui::XmlImporter::getMultitouchStringForBitMask(propertyValue)));
}

void SpriteInspector::addSpritePropertyBlend(const std::wstring& propertyName, const ds::ui::BlendMode& blendMode){
	doAddSpriteProperty(propertyName, ds::wstr_from_utf8(getStringForBlendMode(blendMode)));
}

void SpriteInspector::addSpriteProperty(const std::wstring& propertyName, const ci::Color propertyValue){
	doAddSpriteProperty(propertyName, ds::wstr_from_utf8(ds::ui::XmlImporter::RGBToHex(propertyValue)));
}

void SpriteInspector::addSpriteProperty(const std::wstring& propertyName, const ci::ColorA propertyValue){
	doAddSpriteProperty(propertyName, ds::wstr_from_utf8(ds::ui::XmlImporter::ARGBToHex(propertyValue)));
}

void SpriteInspector::addSpriteProperty(const std::wstring& propertyName, const ci::Vec2f propertyValue){
	std::wstringstream wss;
	wss << propertyValue.x << L", " << propertyValue.y;
	doAddSpriteProperty(propertyName, wss.str());
}

void SpriteInspector::addSpriteProperty(const std::wstring& propertyName, const ci::Vec3f propertyValue){
	std::wstringstream wss;
	wss << propertyValue.x << L", " << propertyValue.y << L", " << propertyValue.z;
	doAddSpriteProperty(propertyName, wss.str());
}

void SpriteInspector::addSpriteProperty(const std::wstring& propertyName, const float propertyValue){
	doAddSpriteProperty(propertyName, ds::value_to_wstring(propertyValue));
}

void SpriteInspector::addSpriteProperty(const std::wstring& propertyName, const int propertyValue){
	doAddSpriteProperty(propertyName, ds::value_to_wstring(propertyValue));
}

void SpriteInspector::addSpritePropertyLayoutSizeMode(const std::wstring& propertyName, const int propertyValue){
	doAddSpriteProperty(propertyName, ds::wstr_from_utf8(ds::ui::XmlImporter::getLayoutSizeModeString(propertyValue)));
}

void SpriteInspector::addSpritePropertyLayoutVAlign(const std::wstring& propertyName, const int propertyValue){
	doAddSpriteProperty(propertyName, ds::wstr_from_utf8(ds::ui::XmlImporter::getLayoutVAlignString(propertyValue)));
}

void SpriteInspector::addSpritePropertyLayoutHAlign(const std::wstring& propertyName, const int propertyValue){
	doAddSpriteProperty(propertyName, ds::wstr_from_utf8(ds::ui::XmlImporter::getLayoutHAlignString(propertyValue)));
}

void SpriteInspector::addSpritePropertyLayoutType(const std::wstring& propertyName, const ds::ui::LayoutSprite::LayoutType& propertyValue){
	doAddSpriteProperty(propertyName, ds::wstr_from_utf8(ds::ui::XmlImporter::getLayoutTypeString(propertyValue)));
}

void SpriteInspector::addSpritePropertyLayoutShrink(const std::wstring& propertyName, const ds::ui::LayoutSprite::ShrinkType& propertyValue){
	std::wstring sizeString = L"none";
	if(propertyValue == ds::ui::LayoutSprite::kShrinkWidth){
		sizeString = L"width";
	} else if(propertyValue == ds::ui::LayoutSprite::kShrinkHeight){
		sizeString = L"height";
	} else if(propertyValue == ds::ui::LayoutSprite::kShrinkBoth){
		sizeString = L"both";
	}
	doAddSpriteProperty(propertyName, sizeString);
}

void SpriteInspector::doAddSpriteProperty(const std::wstring& propertyName, const std::wstring& propertyValue){
	SpritePropertyItem* treeItem = new SpritePropertyItem(mGlobals, propertyName, propertyValue);
	mLayout->addChildPtr(treeItem);
	treeItem->setValueTappedCallback([this](SpritePropertyItem* treeItem){
		setInputField(treeItem);
	});

	mSpriteProperties.push_back(treeItem);
}

void SpriteInspector::layout(){
	if(mLayout){
		mLayout->runLayout();
		setSize(mLayout->getWidth(), mLayout->getHeight() + 20.0f);
	}
}

void SpriteInspector::animateOn(){
	if(mLayout){
		mLayout->tweenAnimateOn(true, 0.0f, 0.1f);
	}
}

void SpriteInspector::animateOff(){
	tweenOpacity(0.0f, mGlobals.getSettingsLayout().getFloat("story_view:anim_time", 0, 0.35f), 0.0f, ci::EaseNone(), [this]{hide(); });
}

void SpriteInspector::setInputField(SpritePropertyItem* field){
	for(auto it = mSpriteProperties.begin(); it < mSpriteProperties.end(); ++it){
		(*it)->getValueField()->setColor(ci::Color::white());
	}


	if(field){
		mEngine.getNotifier().notify(InputFieldSetRequest(field->getValueField()));
		field->getValueField()->setColor(ci::Color(0.4f, 0.4f, 0.8f));
		mCurrentInputTreeItem = field;
	} else {
		mEngine.getNotifier().notify(InputFieldSetRequest(nullptr));
		mCurrentInputTreeItem = nullptr;
	}
}

} // namespace layout_builder
