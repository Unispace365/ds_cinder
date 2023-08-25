#pragma once
#ifndef DS_UI_SPRITE_WEB_H_
#define DS_UI_SPRITE_WEB_H_

#include <cinder/app/KeyEvent.h>
#include <cinder/app/MouseEvent.h>
#include <ds/ui/soft_keyboard/entry_field.h>
#include "ds/ui/sprite/text.h"

#include <mutex>
#include <thread>

namespace ds { namespace ui {



//typedef DummyWebSprite Web;
class Web : public ds::ui::IEntryField {
public:
	struct AuthCallback {
		AuthCallback() : mIsProxy(false), mPort(0) {}
		bool			mIsProxy;
		std::string		mHost;
		int				mPort;
		std::string		mRealm;
		std::string		mScheme;
	};


	Web(ds::ui::SpriteEngine& engine, float width = 0.0f, float height = 0.0f)
		: IEntryField(engine)
	{
		setSize(width, height);
	}
	~Web() {}


	void										sendClick(const ci::vec3& globalClickPos) {}
	void										sendMouseClick(const ci::vec3& globalClickPoint) {}
	void										setAddressChangedFn(const std::function<void(const std::string& new_address)>&) {}
	virtual void								keyPressed(ci::app::KeyEvent& keyEvent) {}
	virtual void								keyPressed(const std::wstring& keyCharacter, const ds::ui::SoftKeyboardDefs::KeyType keyType) {}
	void										loadUrl(const std::wstring& url) {}
	void										loadUrl(const std::string& url) {}
	void										setTitleChangedFn(const std::function<void(const std::wstring& newTitle)>&) {}
	void										executeJavascript(const std::string& theScript, const std::string& debugUrl = "") {}
	//virtual void								onUpdateClient(const ds::UpdateParams&) override {}
	//virtual void								onUpdateServer(const ds::UpdateParams&) override {}
	//virtual void								drawLocalClient() override {};
	void										setDragScrolling(const bool doScrolling) {}
	void										setDragScrollingMinimumFingers(const int numFingers) {}
	void										setDrawWhileLoading(const bool) {}
	void										setNativeTouchInput(const bool doNativeTouch) {}
	void										setLoadingUpdatedCallback(std::function<void(const bool isLoading)> func) {}
	void										setAuthCallback(std::function<void(AuthCallback)> func) {}
	void										setAllowClicks(const bool doAllowClicks) {}
	void										setDocumentReadyFn(const std::function<void(void)>&) {}
	void										setErrorCallback(std::function<void(const std::string&)> func) {}

	void										goBack() {}
	void										goForward() {}
	void										reload(const bool ignoreCache = false) {}
	//bool										isLoading() {}
	//void										stop() {}
	bool										canGoBack() { return false; }
	bool										canGoForward() { return false; }

	void										authCallbackCancel() {}
	void										authCallbackContinue(const std::string& username, const std::string& password) {}


	// OMG ---------------



public:
	static void									installAsServer(ds::BlobRegistry&);
	static void									installAsClient(ds::BlobRegistry&);
	};

} } // namespace ds::ui

#endif // DS_UI_SPRITE_WEB_H_
