#include "ExxonTag_model.h" 


namespace ds {
namespace model {
namespace {
const int							EMPTY_INT = 0;
const unsigned int					EMPTY_UINT = 0;
const float							EMPTY_FLOAT = 0.0f;
const std::wstring					EMPTY_WSTRING;
const ds::Resource					EMPTY_RESOURCE;
const std::vector<FilterableExxonTagRef> EMPTY_FILTERABLEEXXONTAGREF_VECTOR;

}

/**
* \class ds::model::Data
*/
class ExxonTagRef::Data {
public:
	Data(){}

unsigned int mId;
int mType;
std::wstring mName;
std::vector<FilterableExxonTagRef> mFilterableExxonTagRef;


};

ExxonTagRef::ExxonTagRef(){}


const unsigned int& ExxonTagRef::getId() const {
	if(!mData) return EMPTY_UINT; 
	return mData->mId; 
}
const int& ExxonTagRef::getType() const {
	if(!mData) return EMPTY_INT; 
	return mData->mType; 
}
const std::wstring& ExxonTagRef::getName() const {
	if(!mData) return EMPTY_WSTRING; 
	return mData->mName; 
}
const std::vector<FilterableExxonTagRef>& ExxonTagRef::getFilterableExxonTagRef() const {
	if(!mData) return EMPTY_FILTERABLEEXXONTAGREF_VECTOR; 
	return mData->mFilterableExxonTagRef; 
}


ExxonTagRef& ExxonTagRef::setId(const unsigned int& Id){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mId = Id; 
	return *this; 
}
ExxonTagRef& ExxonTagRef::setType(const int& Type){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mType = Type; 
	return *this; 
}
ExxonTagRef& ExxonTagRef::setName(const std::wstring& Name){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mName = Name; 
	return *this; 
}
ExxonTagRef& ExxonTagRef::setFilterableExxonTagRef(const std::vector<FilterableExxonTagRef>& FilterableExxonTagRef){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mFilterableExxonTagRef = FilterableExxonTagRef; 
	return *this; 
}



} // namespace model
} // namespace ds