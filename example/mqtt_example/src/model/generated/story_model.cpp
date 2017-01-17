#include "Story_model.h" 


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
class StoryRef::Data {
public:
	Data(){}

unsigned int mId;
std::wstring mName;
int mFilterableId;


};

StoryRef::StoryRef(){}


const unsigned int& StoryRef::getId() const {
	if(!mData) return EMPTY_UINT; 
	return mData->mId; 
}
const std::wstring& StoryRef::getName() const {
	if(!mData) return EMPTY_WSTRING; 
	return mData->mName; 
}
const int& StoryRef::getFilterableId() const {
	if(!mData) return EMPTY_INT; 
	return mData->mFilterableId; 
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
StoryRef& StoryRef::setFilterableId(const int& FilterableId){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mFilterableId = FilterableId; 
	return *this; 
}


} // !namespace model
} // !namespace ds

