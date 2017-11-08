
#include "location_model.h"


namespace ds {
namespace model {
namespace {
const int							EMPTY_INT = 0;
const unsigned int					EMPTY_UINT = 0;
const float							EMPTY_FLOAT = 0.0f;
const std::wstring					EMPTY_WSTRING;
const ds::Resource					EMPTY_RESOURCE;
const Poco::DateTime				EMPTY_DATE;

}

/**
* \class ds::model::Data
*/
class LocationRef::Data {
public:
	Data()
	: mId(EMPTY_INT)
	, mLat(EMPTY_FLOAT)
	, mLayer(EMPTY_INT)
	, mLong(EMPTY_FLOAT)
	, mName(EMPTY_WSTRING)
	, mPopulation(EMPTY_WSTRING)
{}

int mId;
float mLat;
int mLayer;
float mLong;
std::wstring mName;
std::wstring mPopulation;


};

LocationRef::LocationRef(){}


const int& LocationRef::getId() const {
	if(!mData) return EMPTY_INT; 
	return mData->mId; 
}
const float& LocationRef::getLat() const {
	if(!mData) return EMPTY_FLOAT; 
	return mData->mLat; 
}
const int& LocationRef::getLayer() const {
	if(!mData) return EMPTY_INT; 
	return mData->mLayer; 
}
const float& LocationRef::getLong() const {
	if(!mData) return EMPTY_FLOAT; 
	return mData->mLong; 
}
const std::wstring& LocationRef::getName() const {
	if(!mData) return EMPTY_WSTRING; 
	return mData->mName; 
}
const std::wstring& LocationRef::getPopulation() const {
	if(!mData) return EMPTY_WSTRING; 
	return mData->mPopulation; 
}


LocationRef& LocationRef::setId(const int& Id){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mId = Id; 
	return *this; 
}
LocationRef& LocationRef::setLat(const float& Lat){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mLat = Lat; 
	return *this; 
}
LocationRef& LocationRef::setLayer(const int& Layer){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mLayer = Layer; 
	return *this; 
}
LocationRef& LocationRef::setLong(const float& Long){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mLong = Long; 
	return *this; 
}
LocationRef& LocationRef::setName(const std::wstring& Name){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mName = Name; 
	return *this; 
}
LocationRef& LocationRef::setPopulation(const std::wstring& Population){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mPopulation = Population; 
	return *this; 
}



} // namespace model
} // namespace ds