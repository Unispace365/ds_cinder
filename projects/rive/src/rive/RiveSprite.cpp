#include "stdafx.h"

#include <fstream>

#include <rive/animation/state_machine.hpp>
#include <rive/animation/state_machine_instance.hpp>
#include <rive/animation/state_machine_listener.hpp>
#include <rive/scene.hpp>

#include "rive/RiveRendererNvPath.h"
#include "rive/RiveSprite.h"

namespace ds { namespace ui {

	RiveSprite::RiveSprite(SpriteEngine& engine, const char* filePath)
	  : Sprite(engine) {
		mFactory  = std::make_unique<ds::ui::RiveFactoryNvPath>();
		mRenderer = std::make_unique<ds::ui::RiveRendererNvPath>();

		const auto path	 = ds::Environment::expand(filePath);
		const auto bytes = readFile(path);

		rive::FileAssetLoader* loader = nullptr;
		rive::ImportResult	   result;

		mFile = rive::File::import(bytes, mFactory.get(), &result, loader);
		if (result == rive::ImportResult::success && mFile != nullptr) {
			mArtBoard = mFile->artboardDefault();
			if (mArtBoard) {
				mArtBoard->advance(0.0f);

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

				playAnimation(0);
			}
		}
	}

	RiveSprite::~RiveSprite() = default;

	void RiveSprite::playAnimation(size_t index) {
		if (!mArtBoard || index >= mArtBoard->animationCount()) return;

		mAnimation = mArtBoard->animationAt(index);
		mAnimation->inputCount();

		mAnimation->time(mAnimation->animation()->startSeconds());
		mAnimation->loopValue((int)rive::Loop::loop);
		mAnimation->direction(1);
	}

	void RiveSprite::onUpdateServer(const ds::UpdateParams& updateParams) {
		if (!mArtBoard) return;

		auto scene = mArtBoard->defaultScene();
		if (scene && (mIsMouseMoved || mIsMouseDown || mIsMouseUp)) {
			rive::HitResult hitResult = rive::HitResult::none;
			if (mIsMouseMoved) {
				DS_LOG_INFO("Moved");
				hitResult = scene->pointerMove({mMousePointer.x, mMousePointer.y});
			} else if (mIsMouseDown) {
				DS_LOG_INFO("Down");
				hitResult = scene->pointerDown({mMousePointer.x, mMousePointer.y});
			} else if (mIsMouseUp) {
				DS_LOG_INFO("Up");
				hitResult = scene->pointerUp({mMousePointer.x, mMousePointer.y});
			}

			if (hitResult != rive::HitResult::none) {
			} else {
				DS_LOG_INFO("Exit");
				scene->pointerExit({mMousePointer.x, mMousePointer.y});
			}

			bool needsDraw = scene->advanceAndApply(updateParams.getDeltaTime());
			mIsMouseMoved  = false;
			mIsMouseDown   = false;
			mIsMouseUp	   = false;
		}

		mArtBoard->advance(updateParams.getDeltaTime());
	}

	void RiveSprite::drawLocalClient() {
		if (!mArtBoard || !mRenderer) return;

		nvpath::ScopedPathRendering sp;
		// if (mAnimation)
		//	mAnimation->draw(mRenderer.get());
		// else
		mArtBoard->draw(mRenderer.get());
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