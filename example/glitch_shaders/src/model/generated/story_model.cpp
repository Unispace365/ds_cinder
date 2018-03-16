#include "stdafx.h"

#include "Story_model.h" 


namespace ds {
namespace model {
namespace {
const int							EMPTY_INT = 0;
const unsigned int					EMPTY_UINT = 0;
const float							EMPTY_FLOAT = 0.0f;
const std::wstring					EMPTY_WSTRING;
const ds::Resource					EMPTY_RESOURCE;
const std::vector<MediaRef> EMPTY_MEDIAREF_VECTOR;
const std::vector<StoryRef> EMPTY_STORYREF_VECTOR;

}

/**
* \class ds::model::Data
*/
class StoryRef::Data {
public:
	Data()
	: mBody(EMPTY_WSTRING)
	, mFileName(EMPTY_WSTRING)
	, mId(EMPTY_INT)
	, mMediaRef(EMPTY_MEDIAREF_VECTOR)
	, mPrimaryResource(EMPTY_RESOURCE)
	, mStoryRef(EMPTY_STORYREF_VECTOR)
	, mTitle(EMPTY_WSTRING)
	, mType(EMPTY_WSTRING)
{}

std::wstring mBody;
std::wstring mFileName;
int mId;
ds::Resource mPrimaryResource;
std::wstring mTitle;
std::wstring mType;
std::vector<MediaRef> mMediaRef;
std::vector<StoryRef> mStoryRef;


};

StoryRef::StoryRef(){}


const std::wstring& StoryRef::getBody() const {
	if(!mData) return EMPTY_WSTRING; 
	return mData->mBody; 
}
const std::wstring& StoryRef::getFileName() const {
	if(!mData) return EMPTY_WSTRING; 
	return mData->mFileName; 
}
const int& StoryRef::getId() const {
	if(!mData) return EMPTY_INT; 
	return mData->mId; 
}
const ds::Resource& StoryRef::getPrimaryResource() const {
	if(!mData) return EMPTY_RESOURCE; 
	return mData->mPrimaryResource; 
}
const std::wstring& StoryRef::getTitle() const {
	if(!mData) return EMPTY_WSTRING; 
	return mData->mTitle; 
}
const std::wstring& StoryRef::getType() const {
	if(!mData) return EMPTY_WSTRING; 
	return mData->mType; 
}
const std::vector<MediaRef>& StoryRef::getMediaRef() const {
	if(!mData) return EMPTY_MEDIAREF_VECTOR; 
	return mData->mMediaRef; 
}
const std::vector<StoryRef>& StoryRef::getStoryRef() const {
	if(!mData) return EMPTY_STORYREF_VECTOR; 
	return mData->mStoryRef; 
}


StoryRef& StoryRef::setBody(const std::wstring& Body){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mBody = Body; 
	return *this; 
}
StoryRef& StoryRef::setFileName(const std::wstring& FileName){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mFileName = FileName; 
	return *this; 
}
StoryRef& StoryRef::setId(const int& Id){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mId = Id; 
	return *this; 
}
StoryRef& StoryRef::setPrimaryResource(const ds::Resource& PrimaryResource){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mPrimaryResource = PrimaryResource; 
	return *this; 
}
StoryRef& StoryRef::setTitle(const std::wstring& Title){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mTitle = Title; 
	return *this; 
}
StoryRef& StoryRef::setType(const std::wstring& Type){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mType = Type; 
	return *this; 
}
StoryRef& StoryRef::setMediaRef(const std::vector<MediaRef>& MediaRef){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mMediaRef = MediaRef; 
	return *this; 
}
StoryRef& StoryRef::setStoryRef(const std::vector<StoryRef>& StoryRef){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mStoryRef = StoryRef; 
	return *this; 
}



} // namespace model
} // namespace ds
