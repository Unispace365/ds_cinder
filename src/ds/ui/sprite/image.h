#pragma once
#ifndef DS_IMAGE_H
#define DS_IMAGE_H

#include <string>
#include "ds/ui/sprite/sprite.h"
#include "ds/ui/image_source/image_owner.h"

namespace ds {
namespace ui {

class Image : public Sprite
			, public ImageOwner {
public:
	// Cache any texture loaded by this sprite, never releasing it.
	static const int			IMG_CACHE_F = (1<<0);
	// Begin loading an image as soon as it's received.
	static const int			IMG_PRELOAD_F = (1<<1);

public:
	static Image&				makeImage(SpriteEngine&, const ds::Resource&, Sprite* parent = nullptr);

	Image(SpriteEngine&, const int flags = 0);
	Image(SpriteEngine&, const std::string& filename, const int flags = 0);
	Image(SpriteEngine&, const ds::Resource::Id&, const int flags = 0);
	~Image();

	void						setSize( float width, float height );
	void						setSizeAll( float width, float height, float depth );
	virtual void				updateServer(const UpdateParams&);
	virtual void				drawLocalClient();
	bool						isLoaded() const;
	// Deprecated, see ImageOwner API
#if 1
	void						loadImage( const std::string &filename );
	Image&						setResourceFilename( const std::string &filename );
	Image&						setResourceId(const ds::Resource::Id &resourceId);
	void						clearResource();
#endif
	struct Status {
		static const int		STATUS_EMPTY = 0;
		static const int		STATUS_LOADED = 1;
		int						mCode;
	};
	void						setStatusCallback(const std::function<void(const Status&)>&);

protected:
	virtual void				onImageChanged();
	virtual void				writeAttributesTo(ds::DataBuffer&);
	virtual void				readAttributeFrom(const char attributeId, ds::DataBuffer&);

private:
	typedef Sprite				inherited;

	void						setStatus(const int);
	void						init();

	Status						mStatus;
	bool						mStatusDirty;
	std::function<void(const Status&)>
								mStatusFn;

	// Initialization
public:
	static void					installAsServer(ds::BlobRegistry&);
	static void					installAsClient(ds::BlobRegistry&);
};

} // namespace ui
} // namespace ds

#endif//DS_IMAGE_H
