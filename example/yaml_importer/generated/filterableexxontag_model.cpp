#include "FilterableExxonTag_model.h" 


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
class FilterableExxonTagRef::Data {
public:
	Data(){}

unsigned int mId;
unsigned int mFilterableId;
unsigned int mExxonTagId;


};

FilterableExxonTagRef::FilterableExxonTagRef(){}


const unsigned int& FilterableExxonTagRef::getId() const {
	if(!mData) return EMPTY_UINT; 
	return mData->mId; 
}
const unsigned int& FilterableExxonTagRef::getFilterableId() const {
	if(!mData) return EMPTY_UINT; 
	return mData->mFilterableId; 
}
const unsigned int& FilterableExxonTagRef::getExxonTagId() const {
	if(!mData) return EMPTY_UINT; 
	return mData->mExxonTagId; 
}


FilterableExxonTagRef& FilterableExxonTagRef::setId(const unsigned int& Id){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mId = Id; 
	return *this; 
}
FilterableExxonTagRef& FilterableExxonTagRef::setFilterableId(const unsigned int& FilterableId){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mFilterableId = FilterableId; 
	return *this; 
}
FilterableExxonTagRef& FilterableExxonTagRef::setExxonTagId(const unsigned int& ExxonTagId){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mExxonTagId = ExxonTagId; 
	return *this; 
}



} // namespace model
} // namespace ds