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
				mWidth	= mArtBoard->width();
				mHeight = mArtBoard->height();
				setTransparent(false);

				enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
				enable(true);

				setProcessTouchCallback([this](Sprite* s, const TouchInfo& info) {
					mMousePointer = globalToLocal(info.mCurrentGlobalPoint);
					mIsMouseDown  = info.mPhase == TouchInfo::Added && info.mNumberFingers == 1;
					mIsMouseUp	  = info.mPhase == TouchInfo::Removed && info.mNumberFingers == 1;
				});
			}
		}
	}

	RiveSprite::~RiveSprite() = default;

	void RiveSprite::onUpdateServer(const ds::UpdateParams& updateParams) {
		if (!mArtBoard) return;

		auto animations = mArtBoard->stateMachineCount();
		for (int i = 0; i < animations; i++) {
			auto stateMachine = mArtBoard->stateMachineAt(i);
			if (stateMachine) {
				stateMachine->advance(updateParams.getDeltaTime());
			}
		}

		auto scene = mArtBoard->defaultScene();
		if (scene) {
			if (mIsMouseDown) scene->pointerDown({mMousePointer.x, mMousePointer.y});
			scene->pointerMove({mMousePointer.x, mMousePointer.y});
			if (mIsMouseUp) scene->pointerUp({mMousePointer.x, mMousePointer.y});
		}
		mArtBoard->advance(updateParams.getDeltaTime());
	}

	void RiveSprite::drawLocalClient() {
		if (!mArtBoard) return;

		nvpath::ScopedPathRendering sp;
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