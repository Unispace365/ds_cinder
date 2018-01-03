#include "stdafx.h"

#include "data_model.h" 

#include <ds/util/string_util.h>
#include <ds/util/color_util.h>


namespace ds {
namespace model {
namespace {
const int							EMPTY_INT = 0;
const unsigned int					EMPTY_UINT = 0;
const ci::vec2						EMPTY_VEC2 = ci::vec2();
const ci::vec3						EMPTY_VEC3 = ci::vec3();
const ci::Rectf						EMPTY_RECTF = ci::Rectf();
const float							EMPTY_FLOAT = 0.0f;
const double						EMPTY_DOUBLE = 0.0;
const ci::Color						EMPTY_COLOR = ci::Color(0.0f, 0.0f, 0.0f);
const ci::ColorA					EMPTY_COLORA = ci::ColorA(0.0f, 0.0f, 0.0f, 0.0f);
const std::string					EMPTY_STRING;
const std::wstring					EMPTY_WSTRING;
const ds::Resource					EMPTY_RESOURCE;
const std::vector<DataModelRef>		EMPTY_DATAMODELREF_VECTOR;
const DataModelRef										EMPTY_DATAMODEL;
const std::map<std::string, DataModelRef>				EMPTY_PROPERTY_MAP;
const std::map<std::string, std::vector<DataModelRef>>	EMPTY_CHILDREN_MAP;

}

/**
* \class ds::model::Data
*/
class DataModelRef::Data {
public:
	Data()
		: mValue(EMPTY_STRING)
		, mName(EMPTY_STRING)
		, mId(EMPTY_INT)
		, mResource(EMPTY_RESOURCE)
	{}

	std::string mValue;
	std::string mName;
	int mId;
	ds::Resource mResource;
	std::map<std::string, DataModelRef> mProperties;
	std::map<std::string, std::vector<DataModelRef>> mChildren;

};

DataModelRef::DataModelRef() {}


DataModelRef::DataModelRef(const std::string& value, const std::string& name /*= ""*/, const int id /*= 0*/) {
	setValue(value);
	setName(name);
	setId(id);
}

const int& DataModelRef::getId() const {
	if(!mData) return EMPTY_INT;
	return mData->mId;
}

void DataModelRef::setId(const int& id) {
	createData();
	mData->mId = id;
}

const std::string& DataModelRef::getName() const {
	if(!mData) return EMPTY_STRING;
	return mData->mName;
}

void DataModelRef::setName(const std::string& name) {
	createData();
	mData->mName = name;
}

const bool DataModelRef::empty() const {
	if(!mData) return true;
	if(mData->mId == EMPTY_INT
	   && mData->mValue == EMPTY_STRING
	   && mData->mName == EMPTY_STRING
	   && mData->mChildren.empty()
	   && mData->mProperties.empty()
	   && mData->mResource.empty()
	   ) {
		return true;
	}

	return false;
}

const std::string& DataModelRef::getValue() const {
	if(!mData) return EMPTY_STRING;
	return mData->mValue;
}

void DataModelRef::setValue(const std::string& value) {
	createData();
	mData->mValue = value;
}

void DataModelRef::setValue(const std::wstring& value) {
	createData();
	mData->mValue = ds::utf8_from_wstr(value);
}

void DataModelRef::setValue(const int& value) {
	createData();
	mData->mValue = ds::value_to_string<int>(value);
}

void DataModelRef::setValue(const double& value) {
	createData();
	mData->mValue = ds::value_to_string<double>(value);
}

void DataModelRef::setValue(const float& value) {
	createData();
	mData->mValue = ds::value_to_string<float>(value);
}

void DataModelRef::setValue(const ci::Color& value) {
	createData();
	mData->mValue = ds::unparseColor(value);
}

void DataModelRef::setValue(const ci::ColorA& value) {
	createData();
	mData->mValue = ds::unparseColor(value);
}

void DataModelRef::setValue(const ci::vec2& value) {
	createData();
	mData->mValue = ds::unparseVector(value);
}

void DataModelRef::setValue(const ci::vec3& value) {
	createData();
	mData->mValue = ds::unparseVector(value);
}

void DataModelRef::setValue(const ci::Rectf& value) {
	createData();
	mData->mValue = ds::unparseRect(value);
}

ds::Resource DataModelRef::getResource() const {
	if(!mData) return EMPTY_RESOURCE;
	return mData->mResource;
}

void DataModelRef::setResource(const ds::Resource& resource) {
	createData();
	mData->mResource = resource;
}

bool DataModelRef::getBool() const {
	if(!mData) return false;
	return ds::parseBoolean(mData->mValue);
}

int DataModelRef::getInt() const {
	if(!mData) return EMPTY_INT;
	return ds::string_to_int(mData->mValue);
}

float DataModelRef::getFloat() const {
	if(!mData) return EMPTY_FLOAT;
	return ds::string_to_float(mData->mValue);
}

double DataModelRef::getDouble() const {
	if(!mData) return EMPTY_DOUBLE;
	return ds::string_to_double(mData->mValue);
}

const ci::Color DataModelRef::getColor(ds::ui::SpriteEngine& eng) const {
	if(!mData) return EMPTY_COLOR;
	return ds::parseColor(mData->mValue, eng);
}

const ci::ColorA DataModelRef::getColorA(ds::ui::SpriteEngine& eng) const {
	if(!mData) return EMPTY_COLORA;
	return ds::parseColor(mData->mValue, eng);
}

const std::string& DataModelRef::getString() const {
	return getValue();
}

const std::wstring DataModelRef::getWString() const {
	return ds::wstr_from_utf8(getString());
}

const ci::vec2 DataModelRef::getVec2() const {
	if(!mData) return EMPTY_VEC2;
	return ci::vec2(ds::parseVector(mData->mValue));
}

const ci::vec3 DataModelRef::getVec3() const {
	if(!mData) return EMPTY_VEC3;
	return ds::parseVector(mData->mValue);
}

const ci::Rectf DataModelRef::getRect() const {
	if(!mData) return EMPTY_RECTF;
	return ds::parseRect(mData->mValue);
}

const std::map<std::string, DataModelRef>& DataModelRef::getProperties() {
	if(!mData) return EMPTY_PROPERTY_MAP;
	return mData->mProperties;
}

const ds::model::DataModelRef DataModelRef::getProperty(const std::string& propertyName) {
	if(!mData) return EMPTY_DATAMODEL;
	auto findy = mData->mProperties.find(propertyName);
	if(findy != mData->mProperties.end()) {
		return findy->second;
	}

	return EMPTY_DATAMODEL;
}

void DataModelRef::setProperty(const std::string& propertyName, DataModelRef datamodel) {
	createData();

	mData->mProperties[propertyName] = datamodel;
}

const std::map<std::string, std::vector<DataModelRef>>& DataModelRef::getChildrenMap() {
	if(!mData) return EMPTY_CHILDREN_MAP;
	return mData->mChildren;
}

const std::vector<DataModelRef>& DataModelRef::getChildren(const std::string& childrenName) {
	if(!mData) return EMPTY_DATAMODELREF_VECTOR;
	auto findy = mData->mChildren.find(childrenName);
	if(findy != mData->mChildren.end()) {
		return findy->second;
	}

	return EMPTY_DATAMODELREF_VECTOR;
}

DataModelRef DataModelRef::getChild(const std::string& childName) {
	createData();

	auto findy = mData->mChildren.find(childName);
	if(findy == mData->mChildren.end()
	   || findy->second.empty()
	   ) {
		std::vector<DataModelRef> newChildren;
		newChildren.push_back(DataModelRef("", childName));
		mData->mChildren[childName] = newChildren;
		return newChildren.back();
	} 
	// we checked for end and empty above, so this should always be valid
	return findy->second.front();
}

void DataModelRef::setChild(const std::string& childName, DataModelRef datamodel) {
	createData();

	std::vector<DataModelRef> newChildren;
	newChildren.push_back(DataModelRef("", childName));
	mData->mChildren[childName] = newChildren;
}

void DataModelRef::addChild(const std::string& childName, DataModelRef datamodel) {
	createData();

	auto& findy = mData->mChildren.find(childName);
	if(findy == mData->mChildren.end()) {
		std::vector<DataModelRef> newChildren;
		newChildren.push_back(datamodel);
		mData->mChildren[childName] = newChildren;
	} else {
		findy->second.emplace_back(datamodel);
	}
}

void DataModelRef::setChildren(const std::string& childrenName, std::vector<ds::model::DataModelRef> children) {
	createData();
	mData->mChildren[childrenName] = children;
}

void DataModelRef::printTree(const bool verbose, const std::string& indent) {
	if(empty() || !mData) {
		DS_LOG_INFO(indent << "Empty DataModel.");
	} else {
		DS_LOG_INFO(indent << "DataModel id:" << mData->mId << " name:" << mData->mName << " value:" << mData->mValue);
		if(verbose) {
			if(!mData->mResource.empty()) {
				DS_LOG_INFO(indent << "          resource:" << mData->mResource.getAbsoluteFilePath());
			}

			for (auto it : mData->mProperties){
				DS_LOG_INFO(indent << "          prop:" << it.first << " value:" << it.second.getValue());
			}
		}

		if(!mData->mChildren.empty()) {

			for(auto it : mData->mChildren) {
				DS_LOG_INFO(indent << "          children:" << it.first);
				for(auto cit : it.second) {
					cit.printTree(verbose, indent + "  ");
				}
			}
		}
	}
}

//----------- PRIVATE --------------//
void DataModelRef::createData() {
	if(!mData) mData.reset(new Data());
}

} // namespace model
} // namespace ds
