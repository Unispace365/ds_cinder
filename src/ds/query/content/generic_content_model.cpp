#include "stdafx.h"

#include "generic_content_model.h" 


namespace ds {
namespace model {
namespace {
const int							EMPTY_INT = 0;
const unsigned int					EMPTY_UINT = 0;
const float							EMPTY_FLOAT = 0.0f;
const std::wstring					EMPTY_WSTRING;
const ds::Resource					EMPTY_RESOURCE;

}

/**
* \class Data
*/
class GenericContentRef::Data {
public:
	Data()
		: mCategory(EMPTY_WSTRING)
		, mId(EMPTY_INT)
		, mMedia(EMPTY_RESOURCE)
		, mTextEight(EMPTY_WSTRING)
		, mTextFive(EMPTY_WSTRING)
		, mTextFour(EMPTY_WSTRING)
		, mTextOne(EMPTY_WSTRING)
		, mTextSeven(EMPTY_WSTRING)
		, mTextSix(EMPTY_WSTRING)
		, mTextThree(EMPTY_WSTRING)
		, mTextTwo(EMPTY_WSTRING)
		, mTitle(EMPTY_WSTRING)
	{}

	std::wstring mCategory;
	int mId;
	ds::Resource mMedia;
	std::wstring mTextEight;
	std::wstring mTextFive;
	std::wstring mTextFour;
	std::wstring mTextOne;
	std::wstring mTextSeven;
	std::wstring mTextSix;
	std::wstring mTextThree;
	std::wstring mTextTwo;
	std::wstring mTitle;


};

GenericContentRef::GenericContentRef(){}


const std::wstring& GenericContentRef::getCategory() const {
	if(!mData) return EMPTY_WSTRING;
	return mData->mCategory;
}
const int& GenericContentRef::getId() const {
	if(!mData) return EMPTY_INT;
	return mData->mId;
}
const ds::Resource& GenericContentRef::getMedia() const {
	if(!mData) return EMPTY_RESOURCE;
	return mData->mMedia;
}
const std::wstring& GenericContentRef::getTextEight() const {
	if(!mData) return EMPTY_WSTRING;
	return mData->mTextEight;
}
const std::wstring& GenericContentRef::getTextFive() const {
	if(!mData) return EMPTY_WSTRING;
	return mData->mTextFive;
}
const std::wstring& GenericContentRef::getTextFour() const {
	if(!mData) return EMPTY_WSTRING;
	return mData->mTextFour;
}
const std::wstring& GenericContentRef::getTextOne() const {
	if(!mData) return EMPTY_WSTRING;
	return mData->mTextOne;
}
const std::wstring& GenericContentRef::getTextSeven() const {
	if(!mData) return EMPTY_WSTRING;
	return mData->mTextSeven;
}
const std::wstring& GenericContentRef::getTextSix() const {
	if(!mData) return EMPTY_WSTRING;
	return mData->mTextSix;
}
const std::wstring& GenericContentRef::getTextThree() const {
	if(!mData) return EMPTY_WSTRING;
	return mData->mTextThree;
}
const std::wstring& GenericContentRef::getTextTwo() const {
	if(!mData) return EMPTY_WSTRING;
	return mData->mTextTwo;
}
const std::wstring& GenericContentRef::getTitle() const {
	if(!mData) return EMPTY_WSTRING;
	return mData->mTitle;
}


GenericContentRef& GenericContentRef::setCategory(const std::wstring& Category){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mCategory = Category;
	return *this;
}
GenericContentRef& GenericContentRef::setId(const int& Id){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mId = Id;
	return *this;
}
GenericContentRef& GenericContentRef::setMedia(const ds::Resource& Media){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mMedia = Media;
	return *this;
}
GenericContentRef& GenericContentRef::setTextEight(const std::wstring& TextEight){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mTextEight = TextEight;
	return *this;
}
GenericContentRef& GenericContentRef::setTextFive(const std::wstring& TextFive){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mTextFive = TextFive;
	return *this;
}
GenericContentRef& GenericContentRef::setTextFour(const std::wstring& TextFour){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mTextFour = TextFour;
	return *this;
}
GenericContentRef& GenericContentRef::setTextOne(const std::wstring& TextOne){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mTextOne = TextOne;
	return *this;
}
GenericContentRef& GenericContentRef::setTextSeven(const std::wstring& TextSeven){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mTextSeven = TextSeven;
	return *this;
}
GenericContentRef& GenericContentRef::setTextSix(const std::wstring& TextSix){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mTextSix = TextSix;
	return *this;
}
GenericContentRef& GenericContentRef::setTextThree(const std::wstring& TextThree){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mTextThree = TextThree;
	return *this;
}
GenericContentRef& GenericContentRef::setTextTwo(const std::wstring& TextTwo){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mTextTwo = TextTwo;
	return *this;
}
GenericContentRef& GenericContentRef::setTitle(const std::wstring& Title){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mTitle = Title;
	return *this;
}



} // namespace model
} // namespace ds
