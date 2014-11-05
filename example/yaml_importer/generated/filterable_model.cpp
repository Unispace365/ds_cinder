#include "Filterable_model.h" 


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
class FilterableRef::Data {
public:
	Data(){}

unsigned int mId;
int mCountryId;
int mRegionId;
int mCityId;
int mYear;
float mLongitude;
float mLatitude;
std::vector<FilterableExxonTagRef> mFilterableExxonTagRef;


};

FilterableRef::FilterableRef(){}


const unsigned int& FilterableRef::getId() const {
	if(!mData) return EMPTY_UINT; 
	return mData->mId; 
}
const int& FilterableRef::getCountryId() const {
	if(!mData) return EMPTY_INT; 
	return mData->mCountryId; 
}
const int& FilterableRef::getRegionId() const {
	if(!mData) return EMPTY_INT; 
	return mData->mRegionId; 
}
const int& FilterableRef::getCityId() const {
	if(!mData) return EMPTY_INT; 
	return mData->mCityId; 
}
const int& FilterableRef::getYear() const {
	if(!mData) return EMPTY_INT; 
	return mData->mYear; 
}
const float& FilterableRef::getLongitude() const {
	if(!mData) return EMPTY_FLOAT; 
	return mData->mLongitude; 
}
const float& FilterableRef::getLatitude() const {
	if(!mData) return EMPTY_FLOAT; 
	return mData->mLatitude; 
}
const std::vector<FilterableExxonTagRef>& FilterableRef::getFilterableExxonTagRef() const {
	if(!mData) return EMPTY_FILTERABLEEXXONTAGREF_VECTOR; 
	return mData->mFilterableExxonTagRef; 
}


FilterableRef& FilterableRef::setId(const unsigned int& Id){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mId = Id; 
	return *this; 
}
FilterableRef& FilterableRef::setCountryId(const int& CountryId){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mCountryId = CountryId; 
	return *this; 
}
FilterableRef& FilterableRef::setRegionId(const int& RegionId){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mRegionId = RegionId; 
	return *this; 
}
FilterableRef& FilterableRef::setCityId(const int& CityId){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mCityId = CityId; 
	return *this; 
}
FilterableRef& FilterableRef::setYear(const int& Year){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mYear = Year; 
	return *this; 
}
FilterableRef& FilterableRef::setLongitude(const float& Longitude){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mLongitude = Longitude; 
	return *this; 
}
FilterableRef& FilterableRef::setLatitude(const float& Latitude){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mLatitude = Latitude; 
	return *this; 
}
FilterableRef& FilterableRef::setFilterableExxonTagRef(const std::vector<FilterableExxonTagRef>& FilterableExxonTagRef){
	if(!mData) mData.reset(new Data()); 
	if(mData) mData->mFilterableExxonTagRef = FilterableExxonTagRef; 
	return *this; 
}



} // namespace model
} // namespace ds