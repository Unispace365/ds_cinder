#include "stdafx.h"

#include "media_slideshow.h"


#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/util/ui_utils.h>
#include <ds/util/string_util.h>

#include <ds/data/resource.h>

#include "ds/ui/media/media_interface.h"
#include "ds/ui/media/media_interface_builder.h"
#include "ds/ui/media/media_viewer.h"

#include "ds/ui/media/interface/web_interface.h"

namespace {
class Init {
  public:
	Init() {
		ds::App::AddStartup([](ds::Engine& e) {
			e.registerSpriteImporter("media_slideshow", [](ds::ui::SpriteEngine& enginey) -> ds::ui::Sprite* {
				return new ds::ui::MediaSlideshow(enginey);
			});

			/*

				e.registerSpritePropertySetter("media_player_video_reset_on_complete", [](ds::ui::Sprite&	theSprite,
																						  const std::string& theValue,
																						  const std::string&
			   fileReferrer) { if(auto mediaPlayer = dynamic_cast<ds::ui::MediaPlayer*>(&theSprite)) { auto& mvs =
			   mediaPlayer->getSettings(); mvs.mVideoResetOnComplete = ds::parseBoolean(theValue);
						mediaPlayer->setSettings(mvs);
					} else {
						DS_LOG_WARNING("Tried to set the property media_player_video_reset_on_complete on a
			   non-mediaPlayer sprite"); return;
					}
				});
				*/
		});
	}
};

Init INIT;
} // namespace


namespace ds { namespace ui {

	MediaSlideshow::MediaSlideshow(ds::ui::SpriteEngine& eng)
	  : ds::ui::Sprite(eng)
	  , mHolder(nullptr)
	  , mCurItemIndex(0)
	  , mAnimateDuration(0.35f)
	  , mCurrentInterface(nullptr)
	  , mItemChangedCallback(nullptr)
	  , mAllowLoadAhead(true)
	  , mLetterBoxed(true) {

		mHolder = new ds::ui::Sprite(mEngine);
		addChildPtr(mHolder);
	}

	void MediaSlideshow::clearSlideshow() {
		for (auto it = mViewers.begin(); it < mViewers.end(); ++it) {
			(*it)->release();
		}

		mViewers.clear();

		mCurItemIndex = 0;
	}

	void MediaSlideshow::setMediaSlideshow(const std::vector<ds::Resource>& resources) {
		clearSlideshow();

		mCurItemIndex = 0;

		for (auto it = resources.begin(); it < resources.end(); ++it) {
			MediaViewer* mv = new MediaViewer(mEngine, (*it));
			mv->setSettings(mMediaViewerSettings);
			mv->setDefaultBounds(getWidth(), getHeight());
			mv->setDefaultSize(ci::vec2(getWidth(), getHeight()));
			mHolder->addChildPtr(mv);
			mv->setSwipeCallback([this](ds::ui::Sprite* spr, const ci::vec3& amount) {
				// don't advance if we're zoomed in
				if (spr->getWidth() <= getWidth()) {
					if (amount.x > 20.0f) {
						gotoPrev(false);
					} else if (amount.x < -20.0f) {
						gotoNext(false);
					}
				}
			});
			mv->setAnimateDuration(mAnimateDuration);
			mv->setDoubleTapCallback([this, mv](ds::ui::Sprite* bs, const ci::vec3& pos) {
				callAfterDelay([this] { recenterSlides(); }, 0.01f);
			});
			mViewers.push_back(mv);
		}

		loadCurrentAndAdjacent();
		layout();
		recenterSlides();
	}

	void MediaSlideshow::onSizeChanged() {
		layout();
	}

	// don't change the size of this thing in layout
	void MediaSlideshow::layout() {
		float mssWidth	= getWidth();
		float mssHeight = getHeight();
		float xp		= 0.0f;

		for (auto it : mViewers) {
			ci::vec2 size		  = it->getDefaultSize();
			float	 viewerWidth  = size.x;
			float	 viewerHeight = size.y;
			if (mLetterBoxed) {
				it->setViewerSize(viewerWidth, viewerHeight);
			} else {
				if (viewerWidth < viewerHeight) {
					it->setViewerWidth(mssWidth);
				} else {
					it->setViewerHeight(mssHeight);
				}
			}
			it->setPosition(xp, 0.0f);
			it->setOrigin(it->getPosition());
			it->setBoundingArea(ci::Rectf(xp, 0.0f, xp + mssWidth, 0.0f + mssHeight));
			xp += mssWidth;
		}

		setCurrentInterface();
	}

	// This is basically layout, but animates
	void MediaSlideshow::recenterSlides() {
		float		xp = 0.0f;
		const float w  = getWidth();
		const float h  = getHeight();
		for (auto it : mViewers) {
			if (mLetterBoxed) {
				it->animateToDefaultSize();
				it->tweenPosition(ci::vec3(it->getOrigin().x + (w - it->getDefaultSize().x) * 0.5f,
										   (h - it->getDefaultSize().y) * 0.5f, 0.0f),
								  mAnimateDuration, 0.0f, ci::EaseInOutQuad());
			} else {
				if (it->getWidth() < w) {
					it->animateWidthTo(w);
					it->tweenPosition(ci::vec3(it->getOrigin().x, (h - w / it->getContentAspectRatio()) * 0.5f, 0.0f),
									  mAnimateDuration, 0.0f, ci::EaseInOutQuad());
				} else {
					it->animateHeightTo(h);
					it->tweenPosition(
						ci::vec3(it->getOrigin().x + (w - h * it->getContentAspectRatio()) * 0.5f, 0.0f, 0.0f),
						mAnimateDuration, 0.0f, ci::EaseInOutQuad());
				}
			}
			xp += w;
		}

		if (mItemChangedCallback) {
			mItemChangedCallback(mCurItemIndex, (int)mViewers.size());
		}
	}


	void MediaSlideshow::stopAllContent() {
		for (auto it = mViewers.begin(); it < mViewers.end(); ++it) {
			(*it)->stopContent();
		}
	}

	void MediaSlideshow::gotoItemIndex(const int newIndex) {
		if (!mHolder || mViewers.empty()) return;

		const int numViewers = (int)mViewers.size();

		if (mCurItemIndex > -1 && mCurItemIndex < numViewers && mCurItemIndex != newIndex) {
			// if the current view is in a drag, shut that down
			MediaViewer& currentView = *mViewers[mCurItemIndex];
			currentView.enable(false);

			// fix the position if the center is oddly set
			ci::vec3 positionDelta = currentView.getCenter();
			positionDelta.x *= currentView.getWidth() * currentView.getScale().x;
			positionDelta.y *= currentView.getHeight() * currentView.getScale().y;
			currentView.setPosition(currentView.getPosition() - positionDelta);
			currentView.setCenter(0.0f, 0.0f);

			// tell the current view that we're leaving it
			currentView.exit();
		}

		if (newIndex < 0 || newIndex > numViewers - 1) {
			mCurItemIndex = 0;
		} else {
			mCurItemIndex = newIndex;
		}

		const float destX = -(float)(mCurItemIndex)*getWidth();
		mHolder->tweenPosition(ci::vec3(destX, 0.0f, 0.0f), mAnimateDuration, 0.0f, ci::EaseInOutQuad());

		// tell the new view that we're entering it
		mViewers[mCurItemIndex]->enter();

		setCurrentInterface();
		loadCurrentAndAdjacent();
		recenterSlides();
	}

	void MediaSlideshow::setCurrentInterface() {
		if (mViewers.empty() || mCurItemIndex < 0 || mCurItemIndex > (int)mViewers.size() - 1) return;

		// if there was an interface, get rid of it
		if (mCurrentInterface) {
			mCurrentInterface->release();
			mCurrentInterface = nullptr;
		}

		// see if there's an interface to display
		mCurrentInterface =
			MediaInterfaceBuilder::buildMediaInterface(mEngine, mViewers[mCurItemIndex]->getPlayer(), this);
		if (mCurrentInterface) {
			mCurrentInterface->setPosition(getWidth() / 2.0f - mCurrentInterface->getWidth() / 2.0f,
										   getHeight() - mCurrentInterface->getHeight() - 50.0f);
			mCurrentInterface->animateOn();

			auto webInterface = dynamic_cast<ds::ui::WebInterface*>(mCurrentInterface);
			if (webInterface) {
				webInterface->setKeyboardAllow(mMediaViewerSettings.mWebAllowKeyboard);
				webInterface->setKeyboardKeyScale(mMediaViewerSettings.mWebKeyboardKeyScale);
				webInterface->setAllowTouchToggle(mMediaViewerSettings.mWebAllowTouchToggle);
				webInterface->setKeyboardAbove(mMediaViewerSettings.mWebKeyboardAbove);
			}
		}
	}

	void MediaSlideshow::gotoNext(const bool wrap) {
		if (mViewers.empty()) return;
		int newIndex = mCurItemIndex + 1;
		if (newIndex > (int)mViewers.size() - 1) {
			if (!wrap) {
				// don't do anything if we're not supposed to wrap around
				return;
			}
			newIndex = 0;
		}

		gotoItemIndex(newIndex);
	}

	void MediaSlideshow::gotoPrev(const bool wrap) {
		if (mViewers.empty()) return;
		int newIndex = mCurItemIndex - 1;
		if (newIndex < 0) {
			if (!wrap) {
				// don't do anything if we're not supposed to wrap around
				return;
			}
			newIndex = (int)mViewers.size() - 1;
		}

		gotoItemIndex(newIndex);
	}

	void MediaSlideshow::loadCurrentAndAdjacent() {
		// Simple sanity checks
		int sizey = (int)mViewers.size();
		if (mViewers.empty() || mCurItemIndex < 0 || mCurItemIndex > sizey - 1) return;

		if (!mAllowLoadAhead) {
			auto curItem = mViewers[mCurItemIndex];
			auto curPos	 = curItem->getPosition();
			curItem->initialize();
			curItem->enable(true);
			if (mLetterBoxed) {
				curItem->setPosition(curItem->getPosition().x, (getHeight() - curItem->getDefaultSize().y) * 0.5f);
			} else {

				const float spriteAspect = curItem->getWidth() / curItem->getHeight();
				const float areaAspect	 = getWidth() / getHeight();
				float		destScale	 = curItem->getScale().x;

				if (spriteAspect < areaAspect) {
					destScale = getHeight() / curItem->getHeight();
				} else {
					destScale = getWidth() / curItem->getWidth();
				}


				curItem->setViewerSize(ci::vec2(destScale * curItem->getWidth(), destScale * curItem->getHeight()));

				curItem->setPosition(curItem->getPosition().x - (getWidth() - curItem->getScaleWidth()) * 0.5f,
									 (getHeight() - curItem->getScaleHeight()) * 0.5f);
				curItem->sendToFront();
			}
			return;
		}

		for (int i = 0; i < sizey; i++) {
			if (i == mCurItemIndex		  // This one is the current item
				|| i == mCurItemIndex - 1 // This one is the prev item
				|| i == mCurItemIndex + 1 // This one is the next item
				|| (mCurItemIndex == sizey - 1 &&
					i == 0) // the current one is the last one, so wrap and load the first one
				|| (mCurItemIndex == 0 &&
					i == sizey - 1) // The current one is the first one, so wrap backwards and load the last one
			) {
				mViewers[i]->initialize();
				mViewers[i]->enable(true);
				mViewers[i]->setPosition(mViewers[i]->getPosition().x,
										 (getHeight() - mViewers[i]->getDefaultSize().y) * 0.5f);

			} else {
				mViewers[i]->uninitialize();
			}
		}
	}

	void
	MediaSlideshow::setItemChangedCallback(std::function<void(const int currentItemIndex, const int totalItems)> func) {
		mItemChangedCallback = func;
	}

	void MediaSlideshow::userInputReceived() {
		ds::ui::Sprite::userInputReceived();

		if (mCurrentInterface) {
			mCurrentInterface->animateOn();
		}
	}

	void MediaSlideshow::setMediaViewerSettings(const MediaViewerSettings& settings) {
		mMediaViewerSettings = settings;
	}

	void MediaSlideshow::allowLoadAhead(const bool loadAhead) {
		mAllowLoadAhead = loadAhead;
	}


	void MediaSlideshow::setLetterboxed(const bool letterbox) {
		mLetterBoxed = letterbox;
	}

}} // namespace ds::ui
