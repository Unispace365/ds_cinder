#include "stdafx.h"

#include <fstream>

#include <nvpath/nv_path.h>

#include <rive/animation/state_machine_instance.hpp>
#include <rive/animation/state_machine_listener.hpp>
#include <rive/scene.hpp>

#include "ds/rive/nvpath/rive_factory_nvpath.h"
#include "ds/rive/nvpath/rive_renderer_nvpath.h"

#include "ds/ui/sprite/rive_sprite.h"

namespace ds { namespace ui {

	RiveSprite::RiveSprite(SpriteEngine& engine, const char* filePath)
	  : Sprite(engine) {
		mFactory  = std::make_unique<ds::RiveFactoryNvPath>();
		mRenderer = std::make_unique<ds::RiveRendererNvPath>();

		const auto path	 = ds::Environment::expand(filePath);
		const auto bytes = readFile(path);

		rive::FileAssetLoader* loader = nullptr;
		rive::ImportResult	   result;

		mFile = rive::File::import(bytes, mFactory.get(), &result, loader);
		if (result == rive::ImportResult::success && mFile != nullptr) {
			// TODO allow user to set a specific art board.
			mArtBoard = mFile->artboardDefault();
			if (mArtBoard) {
				// TODO allow user to set a specific state machine.
				mStateMachine = mArtBoard->defaultStateMachine();
				if (mStateMachine) {
					mScene = std::move(mStateMachine);
				} else if (mArtBoard->animationCount()) {
					// TODO allow user to set a specific animation.
					mScene = mArtBoard->animationAt(0);
				}

				if (mScene) {
					mScene->advanceAndApply(0.0f);
				}

				mWidth	= mArtBoard->width();
				mHeight = mArtBoard->height();
				setTransparent(false);

				enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
				enable(true);

				setProcessTouchCallback([this](Sprite* s, const TouchInfo& info) {
					mMousePointer = globalToLocal(info.mCurrentGlobalPoint);

					mIsMouseMoved = info.mPhase == TouchInfo::Moved && info.mNumberFingers == 1;
					mIsMouseDown  = info.mPhase == TouchInfo::Added && info.mNumberFingers == 1;
					mIsMouseUp	  = info.mPhase == TouchInfo::Removed && info.mNumberFingers == 0;
				});
			}
		}
	}

	RiveSprite::~RiveSprite() = default;

	void RiveSprite::playAnimation(size_t index) {
		if (!mArtBoard || index >= mArtBoard->animationCount()) return;

		mAnimation = mArtBoard->animationAt(index);
		mAnimation->inputCount();

		mAnimation->time(mAnimation->animation()->startSeconds());
		mAnimation->loopValue(int(rive::Loop::loop));
		mAnimation->direction(1);
	}

	void RiveSprite::onUpdateServer(const ds::UpdateParams& updateParams) {
		if (!mScene) return;

		if (mIsMouseMoved || mIsMouseDown || mIsMouseUp) {
			rive::HitResult hitResult = rive::HitResult::none;
			if (mIsMouseMoved) {
				hitResult = mScene->pointerMove({mMousePointer.x, mMousePointer.y});
			} else if (mIsMouseDown) {
				hitResult = mScene->pointerDown({mMousePointer.x, mMousePointer.y});
			} else if (mIsMouseUp) {
				hitResult = mScene->pointerUp({mMousePointer.x, mMousePointer.y});
			}

			if (hitResult != rive::HitResult::none) {
			} else {
				mScene->pointerExit({mMousePointer.x, mMousePointer.y});
			}

			mIsMouseMoved = false;
			mIsMouseDown  = false;
			mIsMouseUp	  = false;
		}

		bool needsDraw = mScene->advanceAndApply(updateParams.getDeltaTime());
	}

	void RiveSprite::drawLocalClient() {
		if (!mArtBoard || !mRenderer) return;

		nvpath::ScopedPathRendering sp;
		mArtBoard->draw(mRenderer.get());
	}

	void RiveSprite::onSizeChanged() {
		if (mArtBoard) {
			mArtBoard->width(mWidth);
			mArtBoard->height(mHeight);
		}
	}

	std::vector<uint8_t> RiveSprite::readFile(const std::string& path) {
		std::basic_ifstream<uint8_t> file(path, std::ios_base::binary | std::ios_base::ate);
		if (!file.is_open()) return {};

		std::streamsize		 size = file.tellg();
		std::vector<uint8_t> buffer(size);
		if (size > 0) {
			file.seekg(0, std::ios_base::beg);
			file.read(buffer.data(), size);
		}

		return buffer;
	}


}} // namespace ds::ui