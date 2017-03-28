#include "model_model.h"

namespace ds {
namespace {
const std::wstring					EMPTY_WSTRING;
const std::string					EMPTY_STRING;
const ds::Resource					EMPTY_RESOURCE;
const ci::vec2						EMPTY_VEC2F;
const ModelColumn::Type				EMPTY_COLUMN_TYPE = ModelColumn::Integer;
const ModelRelation::Type			EMPTY_MODEL_RELATION_TYPE = ModelRelation::One;
const std::vector<std::string>		EMPTY_STRING_VECTOR;
const std::vector<ModelColumn>		EMPTY_COLUMN_VECTOR;
const std::vector<ModelRelation>	EMPTY_RELATION_VECTOR;
}

/**
* \class ds::ModelColumn::Data
*/
class ModelColumn::Data {
public:
	Data()
		: mColumnName("")
		, mType(ModelColumn::Invalid)
		, mAutoincrement(false)
		, mPrimary(false)
		, mUnsigned(false)
	{ }

	bool					operator==(const Data& o) const {
		return mColumnName == o.mColumnName;
	}

	std::string				mColumnName;
	ModelColumn::Type		mType;
	bool					mAutoincrement;
	bool					mPrimary;
	bool					mUnsigned;
	std::string				mCustomDataType;
	std::string				mEmptyDataName;
};

ModelColumn::ModelColumn()
{
}

ModelColumn::ModelColumn(const std::string& columnName, const Type& dataType, const bool autoIncrement, const bool isPrimary){
	setColumnName(columnName).setType(dataType).setAutoincrement(autoIncrement).setIsPrimary(isPrimary);
}

const std::string& ModelColumn::getColumnName()const{
	if(!mData) return EMPTY_STRING;
	return mData->mColumnName;
}

const ModelColumn::Type& ModelColumn::getType()const{
	if(!mData) return EMPTY_COLUMN_TYPE;
	return mData->mType;
}

const bool ModelColumn::getAutoincrement()const{
	if(!mData) return false;
	return mData->mAutoincrement;
}

const bool ModelColumn::getIsPrimaryKey()const{
	if(!mData) return false;
	return mData->mPrimary;
}

const std::string& ModelColumn::getCustomDataType()const{
	if(!mData) return EMPTY_STRING;
	return mData->mCustomDataType;
}

const std::string& ModelColumn::getCustomEmptyDataName()const{
	if(!mData) return EMPTY_STRING;
	return mData->mEmptyDataName;
}

const bool ModelColumn::getIsUnsigned()const{
	if(!mData) return false;
	return mData->mUnsigned;
}

ModelColumn& ModelColumn::setColumnName(const std::string& name){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mColumnName = name;
	return *this;
}

ModelColumn& ModelColumn::setType(const Type& dataType){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mType = dataType;
	return *this;
}

ModelColumn& ModelColumn::setAutoincrement(const bool autoInc){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mAutoincrement = autoInc;
	return *this;
}

ModelColumn& ModelColumn::setIsPrimary(const bool isPrimary){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mPrimary = isPrimary;
	return *this;
}

ModelColumn& ModelColumn::setIsUnsigned(const bool isUnsigned){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mUnsigned = isUnsigned;
	return *this;
}

ModelColumn& ModelColumn::setCustomDataType(const std::string& name){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mCustomDataType = name;
	return *this;
}

ModelColumn& ModelColumn::setCustomEmptyDataName(const std::string& name){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mEmptyDataName = name;
	return *this;
}

ModelColumn::Type ModelColumn::getTypeForString(const std::string& typeString){

	if(typeString.find("int") != std::string::npos){
		return ModelColumn::Integer;

	} else if(typeString.find("float") != std::string::npos){
		return ModelColumn::Float;

	// decimal is a special float type for the cms, in the format: decimal(4,1) produces value like 123.4
	} else if(typeString.find("decimal") != std::string::npos){
		return ModelColumn::Float;

	// just map the doubles to floats, who cares, right? There's no way this could ever cause issues later, right?
	} else if(typeString.find("double") != std::string::npos){
		return ModelColumn::Float;

	} else if(typeString.find("string") != std::string::npos){
		return ModelColumn::String;
	} else if(typeString.find("text") != std::string::npos){
		return ModelColumn::String;
	} else if(typeString.find("custom") != std::string::npos){
		return ModelColumn::Custom;
	} else if (typeString.find("date") != std::string::npos){
		return ModelColumn::Date;
	}

	return Invalid;
}


/**
* \class ds::ModelRelation::Data
*/
class ModelRelation::Data {
public:
	Data() { mType = ModelRelation::Invalid;  }

	bool					operator==(const Data& o) const {
		return mLocalColumn == o.mLocalColumn && mForeignColumn == o.mForeignColumn && mForeignTable == o.mForeignTable && mType == o.mType;
	}

	std::string				mLocalColumn;
	std::string				mForeignColumn;
	std::string				mForeignTable;
	ModelRelation::Type		mType;
};

ModelRelation::ModelRelation(){
}

ModelRelation::ModelRelation(const std::string& localColumn, const std::string& foreignColumn, const std::string& foreignTable, const Type& relationType){
	setLocalKeyColumn(localColumn).setForeignKeyColumn(foreignColumn).setForeignKeyTable(foreignTable).setType(relationType);
}

const std::string& ModelRelation::getLocalKeyColumn()const{
	if(!mData) return EMPTY_STRING;
	return mData->mLocalColumn;
}

const std::string& ModelRelation::getForeignKeyColumn()const{
	if(!mData) return EMPTY_STRING;
	return mData->mForeignColumn;
}

const std::string& ModelRelation::getForeignKeyTable()const{
	if(!mData) return EMPTY_STRING;
	return mData->mForeignTable;
}

const ModelRelation::Type& ModelRelation::getType()const{
	if(!mData) return EMPTY_MODEL_RELATION_TYPE;
	return mData->mType;
}

ModelRelation& ModelRelation::setLocalKeyColumn(const std::string& name){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mLocalColumn = name;
	return *this;
}

ModelRelation& ModelRelation::setForeignKeyTable(const std::string& name){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mForeignTable = name;
	return *this;
}

ModelRelation& ModelRelation::setForeignKeyColumn(const std::string& name){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mForeignColumn = name;
	return *this;
}

ModelRelation& ModelRelation::setType(const Type& dataType){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mType = dataType;
	return *this;
}



/**
* \class ds::ModelModel::Data
*/
class ModelModel::Data {
public:
	Data() { }

	bool					operator==(const Data& o) const {
		return mTableName == o.mTableName;
	}

	std::string					mTableName;
	std::string					mCustomInclude;
	std::vector<ModelColumn>	mColumns;
	std::vector<ModelRelation>	mRelations;
	std::vector<std::string>	mResourceColumns;
	std::string					mSortColumn;
};

ModelModel::ModelModel(){
}

ModelModel::ModelModel(const std::string& tableName, const std::vector<ModelColumn>& columns, const std::vector<ModelRelation>& relations, 
					   const std::vector<std::string>& resourceColumns, const std::string& sortColumn){
	
}

const std::string& ModelModel::getTableName()const{
	if(!mData) return EMPTY_STRING;
	return mData->mTableName;
}

const std::string& ModelModel::getCustomInclude()const{
	if(!mData) return EMPTY_STRING;
	return mData->mCustomInclude;
}

const std::vector<ModelColumn>& ModelModel::getColumns(){
	if(!mData) return EMPTY_COLUMN_VECTOR;
	return mData->mColumns;
}

const std::vector<ModelRelation>& ModelModel::getRelations()const{
	if(!mData) return EMPTY_RELATION_VECTOR;
	return mData->mRelations;
}

const std::vector<std::string>& ModelModel::getResourceColumns()const{
	if(!mData) return EMPTY_STRING_VECTOR;
	return mData->mResourceColumns;
}

const std::string& ModelModel::getSortColumn()const{
	if(!mData) return EMPTY_STRING;
	return mData->mSortColumn;
}

ModelModel& ModelModel::setTableName(const std::string& name){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mTableName = name;
	return *this;
}

ModelModel& ModelModel::setCustomInclude(const std::string& customInclude){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mCustomInclude = customInclude;
	return *this;
}

ModelModel& ModelModel::setColumns(const std::vector<ModelColumn>& columns){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mColumns = columns;
	return *this;
}

ModelModel& ModelModel::addColumn(const ModelColumn& column){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mColumns.push_back(column);
	return *this;
}

ModelModel& ModelModel::setRelations(const std::vector<ModelRelation>& relations){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mRelations = relations;
	return *this;
}

ModelModel& ModelModel::setResourceColumns(const std::vector<std::string>& resourceColumns){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mResourceColumns = resourceColumns;
	return *this;
}

ModelModel& ModelModel::setSortColumn(const std::string& sortColumn){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mSortColumn = sortColumn;
	return *this;
}

ModelModel& ModelModel::addRelation(const ModelRelation& relation){
	if(!mData) mData.reset(new Data());
	if(mData) mData->mRelations.push_back(relation);
	return *this;
}

}

