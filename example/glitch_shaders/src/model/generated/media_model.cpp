#include "stdafx.h"

#include "Media_model.h" 


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
* \class ds::model::Data
*/
class MediaRef::Data {
public:
	Data()
	: mId(EMPTY_INT)
	, mPresetId(EMPTY_INT)
{}

int mId;
int mPresetId;


};

MediaRef::MediaRef(){}


const int& MediaRef::getId() const {
	if(!mData) return EMPTY_INT; 
	return mData->mId; 
}
const int& MediaRef::getPresetId() const {
	if(!mData) return EMPTY_INT; 
	return mData->mPresetId; 
}


MediaRef& MediaRef::setId(const int& Id){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mId = Id; 
	return *this; 
}
MediaRef& MediaRef::setPresetId(const int& PresetId){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mPresetId = PresetId; 
	return *this; 
}



} // namespace model
} // namespace ds
