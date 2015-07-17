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

	/// @cond cache flags
	/// @note These really belong elsewhere now, like ImgeSource or probably ImageDefs.

	// Cache any texture loaded by this sprite, never releasing it.
	static const int			IMG_CACHE_F = (1<<0);
	// Begin loading an image as soon as it's received.
	static const int			IMG_PRELOAD_F = (1<<1);
	// Enable mipmapping. This only applies to an image source, so being here is weird.
	static const int			IMG_ENABLE_MIPMAP_F = (1<<2);

	/// @endcond

public:

	/// @cond factory functions
	
	static Image&				makeImage(SpriteEngine&, const std::string& filename, Sprite* parent = nullptr);
	static Image&				makeImage(SpriteEngine&, const ds::Resource&, Sprite* parent = nullptr);
	
	/// @endcond

	
	/// @cond Constructor overloads
	
	/// @brief constructs an image sprite
	Image(SpriteEngine&, const int flags = 0);
	Image(SpriteEngine&, const std::string& filename, const int flags = 0);
	Image(SpriteEngine&, const ds::Resource::Id&, const int flags = 0);
	Image(SpriteEngine&, const ds::Resource& resource, const int flags = 0);
	
	virtual ~Image();

	/// @endcond

public:
	/// @note calls Image::setSizeAll(...) internally.
	/// @see Image::setSizeAll(...)
	void						setSize( float width, float height );

	/// @note sets scale based on size passed. More like a convenience.
	void						setSizeAll( float width, float height, float depth ) override;

	/// @brief returns true if the last requested image is loaded as a texture
	bool						isLoaded() const;

public:
	/// @warning this is deprecated and it inhibits all sort of code
	///          smells and breaks object orientation of this class.
	///          prefer using onImageLoaded() or onImageUnloaded().
	/// @deprecated
	struct Status {
		static const int		STATUS_EMPTY = 0;
		static const int		STATUS_LOADED = 1;
		int						mCode;
	};
	
	/// @warning this is deprecated. Use onImageLoaded() or onImageUnloaded() instead.
	/// @deprecated
	void						setStatusCallback(const std::function<void(const Status&)>&);

protected:

	/// @cond image status callbacks

	virtual void				onImageChanged() override;
	virtual void				onImageLoaded() {}
	virtual void				onImageUnloaded() {}

	/// @endcond

protected:

	/// @cond ds::ui::Sprite overrides

	void						updateServer(const UpdateParams&) override;
	void						drawLocalClient() override;
	void						writeAttributesTo(ds::DataBuffer&) override;
	void						readAttributeFrom(const char attributeId, ds::DataBuffer&) override;

	/// @endcond

private:
	typedef Sprite				inherited;

	void						setStatus(const int);
	void						checkStatus();
	void						doOnImageLoaded();
	void						doOnImageUnloaded();

private:
	Status						mStatus;
	std::function<void(const Status&)>
								mStatusFn;
	struct { ci::Rectf mPerspRect; ci::Rectf mOrthoRect; }
								mDrawRect;
public:

	/// @cond Sprite BLOB registry

	static void					installAsServer(ds::BlobRegistry&);
	static void					installAsClient(ds::BlobRegistry&);

	/// @endcond
};

} // namespace ui
} // namespace ds

#endif//DS_IMAGE_H
