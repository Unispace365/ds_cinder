#include "Story_model.h" 

#include "filterable_model.h"

namespace ds {
namespace model {
namespace {
const int							EMPTY_INT = 0;
const unsigned int					EMPTY_UINT = 0;
const float							EMPTY_FLOAT = 0.0f;
const std::wstring					EMPTY_WSTRING;
const ds::Resource					EMPTY_RESOURCE;
const FilterableRef EMPTY_FILTERABLEREF;

}

/**
* \class ds::model::Data
*/
class StoryRef::Data {
public:
	Data()
	: mFilterableId(EMPTY_FILTERABLEREF)
	, mId(EMPTY_UINT)
	, mName(EMPTY_WSTRING)
{}

int mFilterableId;
unsigned int mId;
std::wstring mName;
FilterableRef mFilterableId;


};

StoryRef::StoryRef(){}


const int& StoryRef::getFilterableId() const {
	if(!mData) return EMPTY_INT; 
	return mData->mFilterableId; 
}
const unsigned int& StoryRef::getId() const {
	if(!mData) return EMPTY_UINT; 
	return mData->mId; 
}
const std::wstring& StoryRef::getName() const {
	if(!mData) return EMPTY_WSTRING; 
	return mData->mName; 
}
const FilterableRef& StoryRef::getFilterableId() const {
	if(!mData) return EMPTY_FILTERABLEREF; 
	return mData->mFilterableId; 
}


StoryRef& StoryRef::setFilterableId(const int& FilterableId){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mFilterableId = FilterableId; 
	return *this; 
}
StoryRef& StoryRef::setId(const unsigned int& Id){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mId = Id; 
	return *this; 
}
StoryRef& StoryRef::setName(const std::wstring& Name){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mName = Name; 
	return *this; 
}
StoryRef& StoryRef::setFilterableId(const FilterableRef& FilterableId){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mFilterableId = FilterableId; 
	return *this; 
}



} // namespace model
} // namespace ds