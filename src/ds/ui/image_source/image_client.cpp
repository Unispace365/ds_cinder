#include "stdafx.h"

#include "image_client.h"

#include "ds/app/app_defs.h"
#include "ds/app/image_registry.h"
#include "ds/data/data_buffer.h"
#include "ds/ui/image_source/image_generator.h"
#include "ds/ui/image_source/image_source.h"
#include "ds/ui/sprite/sprite_engine.h"

namespace ds {
namespace ui {

/**
 * \class ds::ui::ImageClient
 */
ImageClient::ImageClient(ds::ui::SpriteEngine& se)
		: mEngine(se)
		, mGenerator(nullptr) {
}

ImageClient::~ImageClient() {
	clear();
}

void ImageClient::clear() {
	delete mGenerator;
	mGenerator = nullptr;
}

void ImageClient::setSource(const ImageSource& src) {
	if (mGenerator && src.generatorMatches(*mGenerator)) return;

	clear();
	mGenerator = src.newGenerator(mEngine);
}

bool ImageClient::getMetaData(ImageMetaData& d) const {
	if (!mGenerator) return false;
	return mGenerator->getMetaData(d);
}

const ci::gl::TextureRef ImageClient::getImage() {
	if (!mGenerator) return nullptr;
	return mGenerator->getImage();
}

void ImageClient::writeTo(DataBuffer& buf) const {
	if (mGenerator) {
		buf.add(mGenerator->getBlobType());
		mGenerator->writeTo(buf);
	}
	buf.add(ds::TERMINATOR_CHAR);
}

bool ImageClient::readFrom(DataBuffer& buf) {
	clear();

	// If all I have is a terminator, then my source didn't exist.
	if (!buf.canRead<char>()) return false;
	const char			next = buf.read<char>();
	if (next == ds::TERMINATOR_CHAR) return true;
	mGenerator = mEngine.getImageRegistry().makeGenerator(next, mEngine);
	if (!mGenerator) return false;

	const bool		ans = mGenerator->readFrom(buf);

	// Consume the terminator charactor
	while (buf.canRead<char>()) {
		const char	next = buf.read<char>();
		if (next == ds::TERMINATOR_CHAR) return true;
	}
	return true;
}


std::string ImageClient::getImageFilename(){
	if(mGenerator){
		return mGenerator->getImageFilename();
	}

	return "";
}

} // namespace ui
} // namespace ds
