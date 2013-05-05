#include "image_client.h"

#include "ds/app/app_defs.h"
#include "ds/app/image_source_registry.h"
#include "ds/data/data_buffer.h"
#include "ds/ui/image_source/image_source.h"
#include "ds/ui/sprite/sprite_engine.h"

namespace ds {
namespace ui {

/**
 * \class ds::ui::ImageClient
 */
ImageClient::ImageClient(ds::ui::SpriteEngine& se)
	: mEngine(se)
	, mSource(nullptr)
{
}

ImageClient::~ImageClient()
{
	clear();
}

void ImageClient::clear()
{
	delete mSource;
	mSource = nullptr;
}

void ImageClient::setSource(ImageSource* g)
{
	clear();
	mSource = g;
}

const ci::gl::Texture* ImageClient::getImage()
{
	if (!mSource) return nullptr;
	return mSource->getImage();
}

void ImageClient::writeTo(DataBuffer& buf) const
{
	if (mSource) {
		buf.add(mSource->getBlobType());
		mSource->writeTo(buf);
	}
  buf.add(ds::TERMINATOR_CHAR);
}

bool ImageClient::readFrom(DataBuffer& buf)
{
	// If all I have is a terminator, then my source didn't exist.
  if (!buf.canRead<char>()) return false;
	const char		next = buf.read<char>();
	if (next == ds::TERMINATOR_CHAR) return true;
	ImageSource*	src = mEngine.getImageSourceRegistry().make(next, mEngine);
	if (!src) return false;

	setSource(src);
	const bool		ans = src->readFrom(buf);

	// Consume the terminator charactor
	while (buf.canRead<char>()) {
		const char	next = buf.read<char>();
		if (next == ds::TERMINATOR_CHAR) return true;
	}
	return true;
}

} // namespace ui
} // namespace ds
