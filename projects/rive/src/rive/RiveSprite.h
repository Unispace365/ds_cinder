#pragma once

#include <rive/artboard.hpp>
#include <rive/factory.hpp>
#include <rive/file.hpp>
#include <rive/renderer.hpp>

#include "ds/ui/sprite/sprite.h"

namespace ds { namespace ui {

	class RiveSprite : public Sprite {
	  public:
		RiveSprite(SpriteEngine& engine, const char* filePath);
		~RiveSprite() override;

		void playAnimation(size_t index);

		void onUpdateServer(const ds::UpdateParams& updateParams) override;
		
		void drawLocalClient() override;

	  private:
		void onSizeChanged() override;

		static std::vector<uint8_t> readFile(const std::string& path);

		std::unique_ptr<rive::Factory>				   mFactory;
		std::unique_ptr<rive::Renderer>				   mRenderer;
		std::unique_ptr<rive::File>					   mFile;
		std::unique_ptr<rive::ArtboardInstance>		   mArtBoard;
		std::unique_ptr<rive::Scene>				   mScene;
		std::unique_ptr<rive::StateMachineInstance>	   mStateMachine;
		std::unique_ptr<rive::LinearAnimationInstance> mAnimation;

		ci::vec2 mMousePointer{};
		bool	 mIsMouseMoved = false;
		bool	 mIsMouseDown  = false;
		bool	 mIsMouseUp	   = false;
	};

}} // namespace ds::ui