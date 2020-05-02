#pragma once
#ifndef DS_IMAGE_H
#define DS_IMAGE_H

#include <string>

#include <ds/ui/sprite/sprite.h>
#include <ds/util/image_meta_data.h>

namespace ds {
namespace ui {

/*!
 * @class     Image
 * @brief     A Sprite that draws an image on-screen
 */
class Image : public Sprite {
public:
	/// \note These really belong elsewhere now, like ImgeSource or probably ImageDefs.

	/// Cache any texture loaded by this sprite, never releasing it.
	static const int			IMG_CACHE_F = (1<<0);
	/// Begin loading an image as soon as it's received.
	static const int			IMG_PRELOAD_F = (1<<1);
	/// Enable mipmapping. This only applies to an image source, so being here is weird.
	static const int			IMG_ENABLE_MIPMAP_F = (1<<2);
	/// Skip using metadata to size image sprite before loading
	static const int			IMG_SKIP_METADATA_F = (1<<3);

	
	static Image&				makeImage(SpriteEngine&, const std::string& filename, Sprite* parent = nullptr);
	static Image&				makeImage(SpriteEngine&, const ds::Resource&, Sprite* parent = nullptr);
	
	
	/// \brief constructs an image sprite
	Image(SpriteEngine&);
	Image(SpriteEngine&, const std::string& filename, const int flags = 0);
	Image(SpriteEngine&, const ds::Resource::Id&, const int flags = 0);
	Image(SpriteEngine&, const ds::Resource& resource, const int flags = 0);

	virtual ~Image();

	/** Loads an image based on the filename.
	* \param filename is the file path or url to the resource.
	* \param flags provides scope info (i.e. ds::ui::Image::IMG_CACHE_F).
	*/
	void						setImageFile(const std::string& filename, const int flags = 0);

	/** Loads an image based on the resource.
	* \param resource is the resource.
	* \param flags provides scope info (i.e. ds::ui::Image::IMG_CACHE_F).
	*/
	virtual void				setImageResource(const ds::Resource& resource, const int flags = 0);

	/** Loads an image based on the resource id.
	* \param resourceId is the resource.
	* \param flags provides scope info (i.e. ds::ui::Image::IMG_CACHE_F).
	*/
	void						setImageResource(const ds::Resource::Id& resourceId, const int flags = 0);

	/** Loads an image based on the resource.
	*   This is here as a convenience to support the sprite base class
	*   This is effectively setImageResource(resource, 0);
	*/
	virtual void setResource(const ds::Resource& resource) override { setImageResource(resource); }

	/// Returns the absolute image path, even if the image was set by resource 
	const std::string&			getImageFilename() { return mFilename; }

	/// Returns the ds::Resource if it was set as a full resource or as an id. 
	ds::Resource				getImageResource() { return mResource; }

	/// Returns the loaded image, if not loaded returns an empty ref (nullptr) 
	const ci::gl::TextureRef	getImageTexture() {	return mTextureRef; }

	/// Clears the image from this sprite. Removes a reference in the image store if not cached 
	void						clearImage();

	/// \note calls Image::setSizeAll(...) internally.
	/// \see Image::setSizeAll(...)
	void						setSize( float width, float height );

	/// \note sets scale based on size passed. More like a convenience.
	void						setSizeAll( float width, float height, float depth ) override;

	/// \brief returns true if the last requested image is loaded as a texture
	virtual bool				isLoaded() const;

	/// The error message if the image status function returned an error
	const std::string&			getErrorMessage() { return mErrorMsg; }

	/// Uses a shader to crop this image to a circle. By default, uses the bounds of the image for the circle (could be an oval)
	/// If you use your own shader for this sprite, the circle crop won't work
	virtual void				setCircleCrop(bool circleCrop);
	/// Set the specific area to circle crop
	virtual void				setCircleCropRect(const ci::Rectf& rect);
	/// Returns if this is set to crop to a circle
	virtual bool				getCircleCrop(){ return mCircleCropped; }
	/// Enables circle cropping and also automatically centers the rect 
	void						cicleCropAutoCenter(); /// whoopsies
	void						circleCropAutoCenter();
	/// Turns off the above functionality
	void						disableCircleCropCentered();

	struct Status {
		static const int		STATUS_EMPTY = 0;
		static const int		STATUS_LOADED = 1;
		static const int		STATUS_ERRORED = 2;
		int						mCode;
	};

	/// Calls a function when the image is actually loaded, so you can fade the image in or whatever
	/// NOTE: This could return immediately if the image is already loaded
	void						setStatusCallback(const std::function<void(const Status&)>&);

protected:

	virtual void				onBuildRenderBatch() override;

	/// \brief override this if you override isLoaded() and there are additional resources to load after isLoaded() returns true
	virtual bool				isLoadedPrimary() const;

	virtual void				onImageChanged() {};
	virtual void				onImageLoaded() {}
	virtual void				onImageUnloaded() {}

	void						onUpdateServer(const UpdateParams& up) override;
	void						onUpdateClient(const UpdateParams& up) override;
	void						drawLocalClient() override;
	void						writeAttributesTo(ds::DataBuffer& buf) override;
	void						readAttributeFrom(const char attributeId, ds::DataBuffer& buf) override;
	bool						getMetaData(ImageMetaData& d) const;

private:
	void						checkStatus();
	void						imageChanged();
	void						setStatus(const int);
	void						doOnImageLoaded();
	void						doOnImageUnloaded();

	Status						mStatus;
	std::string					mErrorMsg;
	std::function<void(const Status&)>
								mStatusFn;
	struct { ci::Rectf mPerspRect; ci::Rectf mOrthoRect; }
								mDrawRect;

	bool						mCircleCropped;
	bool						mCircleCropCentered;
	ci::gl::TextureRef			mTextureRef;
	std::string					mFilename;
	ds::Resource				mResource;
	int							mFlags;
public:

	static void					installAsServer(ds::BlobRegistry&); ///< Register as server
	static void					installAsClient(ds::BlobRegistry&); ///< Register as client
};

} // namespace ui
} // namespace ds

#endif//DS_IMAGE_H
