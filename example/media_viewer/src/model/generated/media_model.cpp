#include "media_model.h"


namespace ds { namespace model {
	namespace {
		const int					EMPTY_INT	= 0;
		const unsigned int			EMPTY_UINT	= 0;
		const float					EMPTY_FLOAT = 0.0f;
		const std::wstring			EMPTY_WSTRING;
		const ds::Resource			EMPTY_RESOURCE;
		const std::vector<MediaRef> EMPTY_MEDIAREF_VECTOR;

	} // namespace

	/**
	 * \class ds::model::Data
	 */
	class MediaRef::Data {
	  public:
		Data()
		  : mBody(EMPTY_WSTRING)
		  , mId(EMPTY_UINT)
		  , mMediaRef(EMPTY_MEDIAREF_VECTOR)
		  , mParentId(EMPTY_INT)
		  , mPrimaryResource(EMPTY_RESOURCE)
		  , mTitle(EMPTY_WSTRING) {}

		unsigned int		  mId;
		std::wstring		  mTitle;
		std::wstring		  mBody;
		ds::Resource		  mPrimaryResource;
		int					  mParentId;
		std::vector<MediaRef> mMediaRef;
	};

	MediaRef::MediaRef() {}


	const unsigned int& MediaRef::getId() const {
		if (!mData) return EMPTY_UINT;
		return mData->mId;
	}
	const std::wstring& MediaRef::getTitle() const {
		if (!mData) return EMPTY_WSTRING;
		return mData->mTitle;
	}
	const std::wstring& MediaRef::getBody() const {
		if (!mData) return EMPTY_WSTRING;
		return mData->mBody;
	}
	const ds::Resource& MediaRef::getPrimaryResource() const {
		if (!mData) return EMPTY_RESOURCE;
		return mData->mPrimaryResource;
	}
	const int& MediaRef::getParentId() const {
		if (!mData) return EMPTY_INT;
		return mData->mParentId;
	}
	const std::vector<MediaRef>& MediaRef::getMediaRef() const {
		if (!mData) return EMPTY_MEDIAREF_VECTOR;
		return mData->mMediaRef;
	}


	MediaRef& MediaRef::setId(const unsigned int& Id) {
		if (!mData) mData.reset(new Data());
		if (mData) mData->mId = Id;
		return *this;
	}
	MediaRef& MediaRef::setTitle(const std::wstring& Title) {
		if (!mData) mData.reset(new Data());
		if (mData) mData->mTitle = Title;
		return *this;
	}
	MediaRef& MediaRef::setBody(const std::wstring& Body) {
		if (!mData) mData.reset(new Data());
		if (mData) mData->mBody = Body;
		return *this;
	}
	MediaRef& MediaRef::setPrimaryResource(const ds::Resource& PrimaryResource) {
		if (!mData) mData.reset(new Data());
		if (mData) mData->mPrimaryResource = PrimaryResource;
		return *this;
	}
	MediaRef& MediaRef::setParentId(const int& ParentId) {
		if (!mData) mData.reset(new Data());
		if (mData) mData->mParentId = ParentId;
		return *this;
	}
	MediaRef& MediaRef::setMediaRef(const std::vector<MediaRef>& MediaRef) {
		if (!mData) mData.reset(new Data());
		if (mData) mData->mMediaRef = MediaRef;
		return *this;
	}


}} // namespace ds::model
