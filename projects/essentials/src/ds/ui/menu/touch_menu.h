#pragma once
#ifndef DS_UI_MENU_TOUCH_MENU
#define DS_UI_MENU_TOUCH_MENU

#include <utility>

#include <ds/touch/five_finger_cluster.h>
#include <ds/ui/sprite/sprite.h>

namespace ds { namespace ui {
	class ClusterView;

	/*
	 * TouchMenu is the short name for a Five Finger Touch Menu.
	 * This is a menu that appears when 4 or more fingers are placed on the screen at any spot.
	 * When the menu appears, the user slides their whole hand to the selected option and releases.
	 * This class manages the touch clusters (parsing touch input into nearby points in FiveFingerCluster),
	 * then loading a view when enough fingers are onscreen and close enough together (a ClusterView)
	 * The ClusterView is where the actual five finger menu ui is created.
	 *
	 * Note: Intended to be used in Ortho roots only
	 */
	class TouchMenu : public ds::ui::Sprite {
	  public:
		/** A simple construction for setting up each item in the menu. */
		struct MenuItemModel {
			MenuItemModel()
			  : mActivatedCallback(nullptr)
			  , mNormalColor(ci::ColorA::white())
			  , mHighlightColor(ci::ColorA::white()) {}
			MenuItemModel(const std::wstring& titley, const std::string& normalImage = "",
						  const std::string&  highImage = "", std::function<void(ci::vec3)> callback = nullptr,
						  const std::wstring& subtitley = L"", const ci::ColorA& normalColor = ci::ColorA::white(),
						  const ci::ColorA&   highlightColor = ci::ColorA::white())
			  : mActivatedCallback(std::move(callback))
			  , mTitle(titley)
			  , mSubtitle(subtitley)
			  , mIconNormalImage(normalImage)
			  , mIconHighlightedImage(highImage)
			  , mNormalColor(normalColor)
			  , mHighlightColor(highlightColor) {}

			std::function<void(ci::vec3)>
				mActivatedCallback; // Called when this item has been activated, the vec3 is the position of the item
			std::wstring mTitle;	// Label text for the menu item
			std::wstring mSubtitle; // Label text for the menu item (second line, probably smaller)
			std::string	 mIconNormalImage;	   // Path to the normal image for the icon
			std::string mIconHighlightedImage; // Path to the highlighted image for the icon, uses normal image if blank
			ci::ColorA	mNormalColor;
			ci::ColorA	mHighlightColor;
		};

		/** A structure for configuring a touch menu */
		struct TouchMenuConfig {

			float mAnimationDuration{0.35f}; // duration of all animations

			float mItemIconHeight{150.0f};			// The height of the icon for each menu item, in pixels
			float mItemTitlePad{20.0f};				// Distance between the icon and the title in each menu item
			float mItemTitleYPositionPercent{0.5f}; // Vertical Position of the title, as a percentage of the height of
													// the menu item
			float		mItemTitleOpacity{0.5f};		   // Opacity of the title, defaults to 0.5
			float		mItemSubtitleOpacity{0.5f};		   // Opacity of the subtitle, defaults to 0.5
			ci::vec2	mItemTitleResizeLimit{0.0f, 0.0f}; // Resize limit for title and subtitle
			ci::vec2	mItemSize{250.0f, 250.0f};		   // size of each menu item
			std::string mItemTitleTextConfig;			   // The text config for the title of the menu item
			std::string mItemSubtitleTextConfig;		   // The text config for the subtitle of the menu item

			float mClusterRadius{280.0f};		  // How large the overall cluster menu is
			float mClusterDirection{-1.0f};		  // Whether items are placed CW (1) or CCW (-1). Defaults to CCW.
			float mClusterPositionOffset{-90.0f}; // Rotation around the center of the menu the first item is placed at,
												  // in degrees. Depends on the cluster direction. Defaults to -90

			float mClusterSizeThreshold{1000.0f}; // How large the cluster of fingers can be before it is invalidated
			float mClusterDistanceThreshold{1000.0f}; // How far from the start point the cluster can be dragged before
													  // it is invalidated

			int mClusterMinTouchPoints{5}; // The minimum number of touch points needed to register as a cluster,
										   // defaults to 5

			std::string mBackgroundImage;			  // The path to an image
			ci::ColorA	mBackgroundColor{0, 0, 0, 1}; // Color to set the background image
			float		mBackgroundOpacity{0.3f};	  // Max opacity for the background image when the cluster is active
			float		mBackgroundScale{3.0f};		  // Scale of the background image when cluster is active
			ci::vec2	mBackgroundOffset{0.0f, 0.0f}; // Position of the background image when cluster is active
			float		mBackgroundPulseAmount{1.0f};
			BlendMode	mBackgroundBlendMode{NORMAL};

			using ClusterAnimation = enum { kAnimateUp = 0, kAnimateDown, kAnimateLeft, kAnimateRight, kAnimateRadial };

			ClusterAnimation mAnimationStyle{kAnimateUp};
			bool			 mDoClipping{true};

			// Add sprites to cluster view to be on top. Add sprites to the graphic parent to be underneath the menu
			// items Important: use the deactivated callback to remove references to any sprites you've added, or don't
			// retain references at all
			std::function<void(ds::ui::Sprite* clusterView, ds::ui::Sprite* graphicParent)> mActivatedCallback =
				nullptr;
			std::function<void(ds::ui::Sprite* clusterView, ds::ui::Sprite* graphicParent)> mDeactivatedCallback =
				nullptr;
		};

		TouchMenu(ds::ui::SpriteEngine& enginey);

		ds::ui::FiveFingerCluster&		 getFiveFingerCluster() { return mFiveFingerCluster; }
		const ds::ui::FiveFingerCluster& getFiveFingerCluster() const { return mFiveFingerCluster; };

		/** Set a vector of models, one for each item in the menu. Clears any currently active cluster views. */
		void setMenuItemModels(std::vector<MenuItemModel> itemModels);

		/** Sets the configuration for the touch menu. Clears any currently active cluster views. */
		void setMenuConfig(TouchMenuConfig touchMenuConfig);

		/** Send all touch info (straight from the engine in most cases) directly here to be parsed */
		void handleTouchInfo(const ds::ui::TouchInfo& ti);
		/** Send all touch info (straight from the engine in most cases) directly here to be parsed, the sprite
		 * parameter is ignored */
		void handleTouchInfo(ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti);

		/** Starts a menu at the specified location. That menu will timeout or close when an option is tapped. */
		void startTappableMenu(const ci::vec3& globalLocation, const float timeoutSeconds = 10.0f);

		/** Deactive all open menus. This may be jarring to users, so use with care.
			Also, if someone has a hand still on the wall, the menu underneath their hand may reappear*/
		void closeAllMenus();

	  private:
		void				 handleClusterUpdate(const ds::ui::TouchInfo::Phase			   tp,
												 const ds::ui::FiveFingerCluster::Cluster& clustersLastStand);
		void				 clearClusters();
		ds::ui::ClusterView* getClusterView();

		ds::ui::FiveFingerCluster			mFiveFingerCluster;
		std::vector<ds::ui::ClusterView*>	mClusterViews;
		std::map<int, ds::ui::ClusterView*> mClusterLookup;

		std::vector<MenuItemModel> mMenuModels;
		TouchMenuConfig			   mTouchMenuConfig;
	};
}} // namespace ds::ui

#endif
