#pragma once
#ifndef DS_IMAGE_H
#define DS_IMAGE_H

#include <string>

#include <ds/ui/sprite/sprite.h>
#include <ds/ui/image_source/image_owner.h>

namespace ds {
namespace ui {

/*!
 * @class     Image
 * @namespace ds::ui
 * @brief     A Sprite that draws an image on-screen
 */
class Image : public Sprite
			, public ImageOwner {
public:
	/// @note These really belong elsewhere now, like ImgeSource or probably ImageDefs.

	// Cache any texture loaded by this sprite, never releasing it.
	static const int			IMG_CACHE_F = (1<<0);
	// Begin loading an image as soon as it's received.
	static const int			IMG_PRELOAD_F = (1<<1);
	// Enable mipmapping. This only applies to an image source, so being here is weird.
	static const int			IMG_ENABLE_MIPMAP_F = (1<<2);

	
	static Image&				makeImage(SpriteEngine&, const std::string& filename, Sprite* parent = nullptr);
	static Image&				makeImage(SpriteEngine&, const ds::Resource&, Sprite* parent = nullptr);
	
	
	/// @brief constructs an image sprite
	Image(SpriteEngine&);
	Image(SpriteEngine&, const std::string& filename, const int flags = 0);
	Image(SpriteEngine&, const ds::Resource::Id&, const int flags = 0);
	Image(SpriteEngine&, const ds::Resource& resource, const int flags = 0);
	
	virtual ~Image();

	/// @note calls Image::setSizeAll(...) internally.
	/// @see Image::setSizeAll(...)
	void						setSize( float width, float height );

	/// @note sets scale based on size passed. More like a convenience.
	void						setSizeAll( float width, float height, float depth ) override;

	/// @brief returns true if the last requested image is loaded as a texture
	virtual bool				isLoaded() const;

	virtual void				setCircleCrop(bool circleCrop);
	virtual void				setCircleCropRect(const ci::Rectf& rect);
	virtual bool				getCircleCrop(){ return mCircleCropped; }

	struct Status {
		static const int		STATUS_EMPTY = 0;
		static const int		STATUS_LOADED = 1;
		int						mCode;
	};

	/// Calls a function when the image is actually loaded, so you can fade the image in or whatever
	void						setStatusCallback(const std::function<void(const Status&)>&);
	void						checkStatus();

protected:

	virtual void				buildRenderBatch() override;

	/// @brief override this if you override isLoaded() and there are additional resources to load after isLoaded() returns true
	virtual bool				isLoadedPrimary() const;

	/// @cond image status callbacks

	virtual void				onImageChanged() override;
	virtual void				onImageLoaded() {}
	virtual void				onImageUnloaded() {}

	void						updateServer(const UpdateParams&) override;
	void						updateClient(const UpdateParams&) override;
	void						drawLocalClient() override;
	void						writeAttributesTo(ds::DataBuffer&) override;
	void						readAttributeFrom(const char attributeId, ds::DataBuffer&) override;

	/// @endcond

private:
	typedef Sprite				inherited;

	void						setStatus(const int);
	void						doOnImageLoaded();
	void						doOnImageUnloaded();

	Status						mStatus;
	std::function<void(const Status&)>
								mStatusFn;
	struct { ci::Rectf mPerspRect; ci::Rectf mOrthoRect; }
								mDrawRect;

	bool						mCircleCropped;

public:

	/// @cond Sprite BLOB registry

	static void					installAsServer(ds::BlobRegistry&);
	static void					installAsClient(ds::BlobRegistry&);

	/// @endcond
};

} // namespace ui
} // namespace ds

#endif//DS_IMAGE_H
