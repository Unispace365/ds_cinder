#include "sprite_inspector.h"

#include <Poco/LocalDateTime.h>

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/app/event_notifier.h>
#include <ds/debug/logger.h>
#include <ds/util/string_util.h>
#include <ds/ui/sprite/multiline_text.h>

#include "app/app_defs.h"
#include "app/globals.h"
#include "events/app_events.h"
#include <typeinfo>
#include <ds/ui/interface_xml/interface_xml_importer.h>
#include "ui/tree_item.h"

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
			std::cout << "setting value: " << mCurrentInputTreeItem->getPropertyName() << " " << ds::utf8_from_wstr(e.mFullText) << " " << valueText << std::endl;
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

	mTreeItems.clear();
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
	addSpriteProperty(L"class", ds::wstr_from_utf8(typeid(*sp).name()));
	addSpriteProperty(L"size", sp->getSize());
	addSpriteProperty(L"color", sp->getColor());
	addSpriteProperty(L"opacity", sp->getOpacity());
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
		doAddSpriteProperty(L"-------------------------", L"-------------------------");
		addSpritePropertyLayoutType(L"layout_type", ls->getLayoutType());
		addSpriteProperty(L"layout_spacing", ls->getSpacing());
		addSpritePropertyLayoutShrink(L"shrink_to_children", ls->getShrinkToChildren());
		addSpritePropertyLayoutVAlign(L"overall_alignment", ls->getOverallAlignment());
	}

	ds::ui::Text* texty = dynamic_cast<ds::ui::Text*>(mLinkedSprite);
	if(texty){
		doAddSpriteProperty(L"-------------------------", L"-------------------------");
		addSpriteProperty(L"text", texty->getText());
		addSpriteProperty(L"font", ds::wstr_from_utf8(texty->getFontFileName()));
		addSpriteProperty(L"font_size", texty->getFontSize());
	}

	ds::ui::MultilineText* multitexty = dynamic_cast<ds::ui::MultilineText*>(mLinkedSprite);
	if(multitexty){
		addSpriteProperty(L"resize_limit", ci::Vec2f(multitexty->getResizeLimitWidth(), multitexty->getResizeLimitHeight()));
		addSpritePropertyLayoutHAlign(L"text_align", multitexty->getAlignment());
	}

	layout();
	animateOn();
}

std::string RGBToHex(int rNum, int gNum, int bNum){
	std::string result;
	char r[255];
	sprintf_s(r, "%.2X", rNum);
	result.append(r);
	char g[255];
	sprintf_s(g, "%.2X", gNum);
	result.append(g);
	char b[255];
	sprintf_s(b, "%.2X", bNum);
	result.append(b);
	return result;
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
	doAddSpriteProperty(propertyName, ds::wstr_from_utf8(RGBToHex((int)(propertyValue.r * 255.0f), (int)(propertyValue.g * 255.0f), (int)(propertyValue.b * 255.0f))));
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
	std::wstring sizeString = L"fixed";
	if(propertyValue == ds::ui::LayoutSprite::kFlexSize){
		sizeString = L"flex";
	} else if(propertyValue == ds::ui::LayoutSprite::kStretchSize){
		sizeString = L"stretch";
	}
	doAddSpriteProperty(propertyName, sizeString);
}

void SpriteInspector::addSpritePropertyLayoutVAlign(const std::wstring& propertyName, const int propertyValue){
	std::wstring sizeString = L"top";
	if(propertyValue == ds::ui::LayoutSprite::kMiddle){
		sizeString = L"middle";
	} else if(propertyValue == ds::ui::LayoutSprite::kBottom){
		sizeString = L"bottom";
	}
	doAddSpriteProperty(propertyName, sizeString);
}

void SpriteInspector::addSpritePropertyLayoutHAlign(const std::wstring& propertyName, const int propertyValue){
	std::wstring sizeString = L"left";
	if(propertyValue == ds::ui::LayoutSprite::kCenter){
		sizeString = L"center";
	} else if(propertyValue == ds::ui::LayoutSprite::kRight){
		sizeString = L"right";
	}
	doAddSpriteProperty(propertyName, sizeString);
}

void SpriteInspector::addSpritePropertyLayoutType(const std::wstring& propertyName, const ds::ui::LayoutSprite::LayoutType& propertyValue){
	std::wstring sizeString = L"none";
	if(propertyValue == ds::ui::LayoutSprite::kLayoutVFlow){
		sizeString = L"vert";
	} else if(propertyValue == ds::ui::LayoutSprite::kLayoutHFlow){
		sizeString = L"horiz";
	} else if(propertyValue == ds::ui::LayoutSprite::kLayoutSize){
		sizeString = L"size";
	}
	doAddSpriteProperty(propertyName, sizeString);
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
	TreeItem* treeItem = new TreeItem(mGlobals, propertyName, propertyValue);
	mLayout->addChildPtr(treeItem);
	treeItem->setValueTappedCallback([this](TreeItem* treeItem){
		setInputField(treeItem);
	});

	mTreeItems.push_back(treeItem);
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

void SpriteInspector::setInputField(TreeItem* field){
	for(auto it = mTreeItems.begin(); it < mTreeItems.end(); ++it){
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
