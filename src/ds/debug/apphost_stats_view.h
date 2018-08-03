#pragma once
#ifndef DS_DEBUG_APPHOST_STATS_VIEW_H_
#define DS_DEBUG_APPHOST_STATS_VIEW_H_

#include "ds/ui/layout/layout_sprite.h"
#include "ds/ui/sprite/text.h"

#include "ds/network/https_client.h"

namespace ds {
namespace ui {

/**
* \class AppHostStatsView
* \brief Show the status of DSAppHost and send it some messages
*/
class AppHostStatsView : public ds::ui::LayoutSprite {
public:

	AppHostStatsView(ds::ui::SpriteEngine&);

	void						activate();
	void						deactivate();
	void						updateStats();
	void						updateText();

private:
	void						addButton(const std::string& str, const std::string& api, bool needsConfirm);
	void						setToConfirm(const std::string str, const std::string api, ds::ui::Text* btnText);
	void						setToAskToConfirm(const std::string str, const std::string api, ds::ui::Text* btnText);
	ds::ui::Text*				getSomeText();
	void						addButtons();
	void						removeButtons();

	ds::ui::Text*				mText;
	ds::net::HttpsRequest		mHttpsRequest;
	std::string					mStatus;

	std::vector<ds::ui::Text*>	mButtons;

	float mPad;


};

} // namespace ui
} // namespace ds

#endif
