#pragma once
#ifndef DS_DEBUG_APPHOST_STATS_VIEW_H_
#define DS_DEBUG_APPHOST_STATS_VIEW_H_

#include "ds/ui/layout/layout_sprite.h"
#include "ds/ui/sprite/text.h"

#include "ds/network/https_client.h"

namespace ds { namespace ui {

	/**
	 * \class AppHostStatsView
	 * \brief Show the status of DSAppHost and send it some messages
	 */
	class AppHostStatsView : public LayoutSprite {
	  public:
		AppHostStatsView(SpriteEngine&);

		void activate();
		void deactivate();
		void updateStats();
		void updateText();

	  private:
		void  addButton(const std::string& str, const std::string& api, bool needsConfirm);
		void  setToConfirm(const std::string& str, const std::string& api, Text* btnText);
		void  setToAskToConfirm(const std::string& str, const std::string& api, Text* btnText);
		Text* getSomeText();
		void  addButtons();
		void  removeButtons();

		Text*			  mText = nullptr;
		net::HttpsRequest mHttpsRequest;
		std::string		  mStatus;

		std::vector<Text*> mButtons;

		float mPad{0};
	};

}} // namespace ds::ui

#endif
