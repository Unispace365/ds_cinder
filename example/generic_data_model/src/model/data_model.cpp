#include "stdafx.h"

#include "data_model.h" 

#include <ds/util/string_util.h>
#include <ds/util/color_util.h>


namespace ds {
namespace model {
namespace {
const int							EMPTY_INT = 0;
const std::string					EMPTY_STRING;
const ds::Resource					EMPTY_RESOURCE;
const std::vector<DataModelRef>		EMPTY_DATAMODELREF_VECTOR;
const DataModelRef										EMPTY_DATAMODEL;
const DataProperty										EMPTY_PROPERTY;
const std::map<std::string, DataProperty>				EMPTY_PROPERTY_MAP;

}

DataProperty::DataProperty()
	: mName("")
	, mValue("")
	, mResource(EMPTY_RESOURCE)
{
}

DataProperty::DataProperty(const std::string& name, const std::string& value) {
	setValue(value);
	setName(name);
}
const std::string& DataProperty::getName() const {
	return mName;
}

void DataProperty::setName(const std::string& name) {
	mName = name;
}

const std::string& DataProperty::getValue() const {
	return mValue;
}

void DataProperty::setValue(const std::string& value) {
	mValue = value;
}

void DataProperty::setValue(const std::wstring& value) {
	mValue = ds::utf8_from_wstr(value);
}

void DataProperty::setValue(const int& value) {
	mValue = ds::value_to_string<int>(value);
}

void DataProperty::setValue(const double& value) {
	mValue = ds::value_to_string<double>(value);
}

void DataProperty::setValue(const float& value) {
	mValue = ds::value_to_string<float>(value);
}

void DataProperty::setValue(const ci::Color& value) {
	mValue = ds::unparseColor(value);
}

void DataProperty::setValue(const ci::ColorA& value) {
	mValue = ds::unparseColor(value);
}

void DataProperty::setValue(const ci::vec2& value) {
	mValue = ds::unparseVector(value);
}

void DataProperty::setValue(const ci::vec3& value) {
	mValue = ds::unparseVector(value);
}

void DataProperty::setValue(const ci::Rectf& value) {
	mValue = ds::unparseRect(value);
}

ds::Resource DataProperty::getResource() const {
	return mResource;
}

void DataProperty::setResource(const ds::Resource& resource) {
	mResource = resource;
}

bool DataProperty::getBool() const {
	return ds::parseBoolean(mValue);
}

int DataProperty::getInt() const {
	return ds::string_to_int(mValue);
}

float DataProperty::getFloat() const {
	return ds::string_to_float(mValue);
}

double DataProperty::getDouble() const {
	return ds::string_to_double(mValue);
}

const ci::Color DataProperty::getColor(ds::ui::SpriteEngine& eng) const {
	return ds::parseColor(mValue, eng);
}

const ci::ColorA DataProperty::getColorA(ds::ui::SpriteEngine& eng) const {
	return ds::parseColor(mValue, eng);
}

const std::string& DataProperty::getString() const {
	return getValue();
}

const std::wstring DataProperty::getWString() const {
	return ds::wstr_from_utf8(getString());
}

const ci::vec2 DataProperty::getVec2() const {
	return ci::vec2(ds::parseVector(mValue));
}

const ci::vec3 DataProperty::getVec3() const {
	return ds::parseVector(mValue);
}

const ci::Rectf DataProperty::getRect() const {
	return ds::parseRect(mValue);
}



class DataModelRef::Data {
public:
	Data()
		: mName(EMPTY_STRING)
		, mLabel(EMPTY_STRING)
		, mId(EMPTY_INT)
	{}

	std::string mName;
	std::string mLabel;
	int mId;
	std::map<std::string, DataProperty> mProperties;
	std::vector<DataModelRef> mChildren;

};

DataModelRef::DataModelRef() {}


DataModelRef::DataModelRef(const std::string& name, const int id, const std::string& label) {
	setName(name);
	setId(id);
	setLabel(label);
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

const std::string& DataModelRef::getLabel() const {
	if(!mData) return EMPTY_STRING;
	return mData->mLabel;
}

void DataModelRef::setLabel(const std::string& name) {
	createData();
	mData->mLabel = name;
}

const bool DataModelRef::empty() const {
	if(!mData) return true;
	if(mData->mId == EMPTY_INT
	   && mData->mName == EMPTY_STRING
	   && mData->mChildren.empty()
	   && mData->mProperties.empty()
	   ) {
		return true;
	}

	return false;
}


const std::map<std::string, DataProperty>& DataModelRef::getProperties() {
	if(!mData) return EMPTY_PROPERTY_MAP;
	return mData->mProperties;
}

void DataModelRef::setProperties(const std::map<std::string, DataProperty>& newProperties) {
	createData();
	mData->mProperties = newProperties;
}

const ds::model::DataProperty DataModelRef::getProperty(const std::string& propertyName) {
	if(!mData) return EMPTY_PROPERTY;
	auto findy = mData->mProperties.find(propertyName);
	if(findy != mData->mProperties.end()) {
		return findy->second;
	}

	return EMPTY_PROPERTY;
}

const std::string DataModelRef::getPropertyValue(const std::string& propertyName) {
	return getProperty(propertyName).getValue();
}

bool DataModelRef::getPropertyBool(const std::string& propertyName){
	return getProperty(propertyName).getBool();
}

int DataModelRef::getPropertyInt(const std::string& propertyName) {
	return getProperty(propertyName).getInt();
}

float DataModelRef::getPropertyFloat(const std::string& propertyName) {
	return getProperty(propertyName).getFloat();
}

double DataModelRef::getPropertyDouble(const std::string& propertyName) {
	return getProperty(propertyName).getDouble();
}

const ci::Color DataModelRef::getPropertyColor(ds::ui::SpriteEngine& eng, const std::string& propertyName) {
	return getProperty(propertyName).getColor(eng);
}

const ci::ColorA DataModelRef::getPropertyColorA(ds::ui::SpriteEngine& eng, const std::string& propertyName) {
	return getProperty(propertyName).getColorA(eng);
}

const std::string DataModelRef::getPropertyString(const std::string& propertyName) {
	return getProperty(propertyName).getString();
}

const std::wstring DataModelRef::getPropertyWString(const std::string& propertyName) {
	return getProperty(propertyName).getWString();
}

const ci::vec2 DataModelRef::getPropertyVec2(const std::string& propertyName) {
	return getProperty(propertyName).getVec2();
}

const ci::vec3 DataModelRef::getPropertyVec3(const std::string& propertyName) {
	return getProperty(propertyName).getVec3();
}

const ci::Rectf DataModelRef::getPropertyRect(const std::string& propertyName) {
	return getProperty(propertyName).getRect();
}

void DataModelRef::setProperty(const std::string& propertyName, DataProperty datamodel) {
	createData();

	mData->mProperties[propertyName] = datamodel;
}

void DataModelRef::setProperty(const std::string& propertyName, const std::string& propertyValue) {
	createData();
	mData->mProperties[propertyName] = DataProperty(propertyName, propertyValue);
}

void DataModelRef::setProperty(const std::string& propertyName, const std::wstring& value) {
	DataProperty dp;
	dp.setName(propertyName);
	dp.setValue(value);
	setProperty(propertyName, dp);
}

void DataModelRef::setProperty(const std::string& propertyName, const int& value) {
	DataProperty dp;
	dp.setName(propertyName);
	dp.setValue(value);
	setProperty(propertyName, dp);
}

void DataModelRef::setProperty(const std::string& propertyName, const double& value) {
	DataProperty dp;
	dp.setName(propertyName);
	dp.setValue(value);
	setProperty(propertyName, dp);
}

void DataModelRef::setProperty(const std::string& propertyName, const float& value) {
	DataProperty dp;
	dp.setName(propertyName);
	dp.setValue(value);
	setProperty(propertyName, dp);
}

void DataModelRef::setProperty(const std::string& propertyName, const ci::Color& value) {
	DataProperty dp;
	dp.setName(propertyName);
	dp.setValue(value);
	setProperty(propertyName, dp);
}

void DataModelRef::setProperty(const std::string& propertyName, const ci::ColorA& value) {
	DataProperty dp;
	dp.setName(propertyName);
	dp.setValue(value);
	setProperty(propertyName, dp);
}

void DataModelRef::setProperty(const std::string& propertyName, const ci::vec2& value) {
	DataProperty dp;
	dp.setName(propertyName);
	dp.setValue(value);
	setProperty(propertyName, dp);
}

void DataModelRef::setProperty(const std::string& propertyName, const ci::vec3& value) {
	DataProperty dp;
	dp.setName(propertyName);
	dp.setValue(value);
	setProperty(propertyName, dp);
}

void DataModelRef::setProperty(const std::string& propertyName, const ci::Rectf& value) {
	DataProperty dp;
	dp.setName(propertyName);
	dp.setValue(value);
	setProperty(propertyName, dp);
}

const std::vector<DataModelRef>& DataModelRef::getChildren() const {
	if(!mData) return EMPTY_DATAMODELREF_VECTOR;
	return  mData->mChildren;
}

DataModelRef DataModelRef::getChild(const size_t index) {
	createData();

	if(index < mData->mChildren.size()) {
		return mData->mChildren[index];
	} else if(!mData->mChildren.empty()) {
		return mData->mChildren.back();
	}

	return EMPTY_DATAMODEL;
}

DataModelRef DataModelRef::getChildById(const int id) {
	createData();

	for(auto it : mData->mChildren) {
		if(it.getId() == id) return it;
	}

	return EMPTY_DATAMODEL;
}

DataModelRef DataModelRef::getChildByName(const std::string& childName) {
	createData();

	if(childName.find(".") != std::string::npos) {
		auto childrens = ds::split(childName, ".", true);
		if(childrens.empty()) {
			DS_LOG_WARNING("DataModelRef::getChild() Cannot find a child with the name \".\"");
		} else {
			DataModelRef curChild = getChildByName(childrens.front());
			for(int i = 1; i < childrens.size(); i++) {
				curChild = curChild.getChildByName(childrens[i]);
			}
			return curChild;
		}
	}

	for (auto it : mData->mChildren){
		if(it.getName() == childName) return it;
	}

	return EMPTY_DATAMODEL;
}

void DataModelRef::addChild(DataModelRef datamodel, const size_t index) {
	createData();

	if(index < mData->mChildren.size()) {
		mData->mChildren.insert(mData->mChildren.begin() + index, datamodel);
	} else {
		mData->mChildren.emplace_back(datamodel);
	}
}

bool DataModelRef::hasChild(const std::string& name) {
	if(!mData || mData->mChildren.empty()) return false;

	for (auto it : mData->mChildren){
		if(it.getName() == name) return true;
	}

	return false;
}

bool DataModelRef::hasChildren() {
	if(!mData) return false;
	return !mData->mChildren.empty();
}

void DataModelRef::setChildren(std::vector<ds::model::DataModelRef> children) {
	createData();
	mData->mChildren = children;
}

void DataModelRef::printTree(const bool verbose, const std::string& indent) {
	if(empty() || !mData) {
		DS_LOG_INFO(indent << "Empty DataModel.");
	} else {
		DS_LOG_INFO(indent << "DataModel id:" << mData->mId << " name:" << mData->mName);
		if(verbose) {

			for(auto it : mData->mProperties) {
				if(!it.second.getResource().empty()) {
					DS_LOG_INFO(indent << "          resource:" << it.second.getResource().getAbsoluteFilePath());
				} else {
					DS_LOG_INFO(indent << "          prop:" << it.first << " value:" << it.second.getValue());
				}
			}
		}

		if(!mData->mChildren.empty()) {

			for(auto it : mData->mChildren) {
			//	DS_LOG_INFO(indent << "          child:" << it.getName());
				it.printTree(verbose, indent + "  ");
				
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
