#include "stdafx.h"

#include <utils/no_op_factory.hpp>
#include <utils/no_op_renderer.hpp>

#include "rive/RiveSprite.h"

namespace ds { namespace ui {

	RiveSprite::RiveSprite(SpriteEngine& engine, const char* filePath)
	  : Sprite(engine) {
		mFactory  = std::make_unique<rive::NoOpFactory>();
		mRenderer = std::make_unique<rive::NoOpRenderer>();

		const auto path	 = ds::Environment::expand(filePath);
		const auto bytes = readFile(path);

		rive::FileAssetLoader* loader = nullptr;
		rive::ImportResult	   result;

		mFile = rive::File::import(bytes, mFactory.get(), &result, loader);
		assert(result == rive::ImportResult::success);
		assert(mFile != nullptr);
		assert(mFile->artboard() != nullptr);
	}

	RiveSprite::~RiveSprite() = default;

	void RiveSprite::onUpdateServer(const ds::UpdateParams& updateParams) {
		mFile->artboard()->advance(updateParams.getElapsedTime());
	}

	void RiveSprite::drawLocalClient() {
		// mArtBoard.draw(mRenderer);
	}

	std::vector<uint8_t> RiveSprite::readFile(const std::string& path) {
		FILE* fp = fopen(path.c_str(), "rb");
		assert(fp != nullptr);

		fseek(fp, 0, SEEK_END);
		const size_t length = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		std::vector<uint8_t> bytes(length);
		assert(fread(bytes.data(), 1, length, fp) == length);
		fclose(fp);

		return bytes;
	}


}} // namespace ds::ui