#pragma once
#ifndef DS_MODEL_MODEL_MODEL
#define DS_MODEL_MODEL_MODEL

#include <ds/data/resource.h>
#include <memory>
#include <vector>
#include <cinder/Vector.h>

namespace ds {

/**
* \class ds::ModelColumn
*			Describes a single column in a table
*/
class ModelColumn {
public:
	typedef enum {Integer = 0, UnsignedInt, Float, String, Resource, Date, Custom, Invalid} Type;

	ModelColumn();
	ModelColumn(const std::string& columnName, const Type& dataType, const bool autoIncrement, const bool isPrimary);

	static Type				getTypeForString(const std::string& typeString);

	const std::string&		getColumnName() const;
	const Type&				getType() const;
	const bool				getAutoincrement() const;
	const bool				getIsPrimaryKey() const;
	const bool				getIsUnsigned() const;
	const std::string&		getCustomDataType() const;
	const std::string&		getCustomEmptyDataName() const;

	ModelColumn&			setColumnName(const std::string& name);
	ModelColumn&			setType(const Type& dataType);
	ModelColumn&			setAutoincrement(const bool autoInc);
	ModelColumn&			setIsPrimary(const bool isPrimary);
	ModelColumn&			setIsUnsigned(const bool isUnsigned);
	ModelColumn&			setCustomDataType(const std::string& dataType); // eg. std::vector<ds::Resource>
	ModelColumn&			setCustomEmptyDataName(const std::string& emptyName); // eg. EMPTY_RESOURCE_VECTOR

private:
	class Data;
	std::shared_ptr<Data>	mData;
};

/**
* \class ds::ModelRelation
*			Describes the relationship between two tables. Both sides of the relationship have a link
*/
class ModelRelation {
public:
	typedef enum {One = 0, Many, Invalid} Type;

	ModelRelation();
	ModelRelation(const std::string& localColumn, const std::string& foreignColumn, const std::string& foreignTable, const Type& relationType);

	const std::string&		getLocalKeyColumn() const;
	const std::string&		getForeignKeyColumn() const;
	const std::string&		getForeignKeyTable() const;
	const Type&				getType() const;


	ModelRelation&			setLocalKeyColumn(const std::string& name);
	ModelRelation&			setForeignKeyColumn(const std::string& name);
	ModelRelation&			setForeignKeyTable(const std::string& name);
	ModelRelation&			setType(const Type& dataType);

private:
	class Data;
	std::shared_ptr<Data>	mData;
};

/**
* \class ds::ModelModel
*			A model for modeling data models. A model's model, if you will. 
*			Question: But why male models?
*/
class ModelModel {
public:
	ModelModel();
	ModelModel(const std::string& tableName, const std::vector<ModelColumn>& columns, const std::vector<ModelRelation>& relations, const std::vector<std::string>& resourceColumns, const std::string& sortColumn);

	bool								operator==(const ModelModel&) const;
	bool								empty() const;
	void								clear();

	const std::string&					getTableName() const;
	const std::string&					getCustomInclude() const;
	const std::string&					getCustomImpInclude() const;
	const std::vector<ModelColumn>&		getColumns();
	const std::vector<ModelRelation>&	getRelations() const;
	const std::vector<std::string>&		getResourceColumns() const;
	const std::string&					getSortColumn() const; // an empty string means no sorting

	ModelModel&							setTableName(const std::string& tableName);
	ModelModel&							setCustomInclude(const std::string& customInclude);
	ModelModel&							setCustomImpInclude(const std::string& customImpInclude);
	ModelModel&							setColumns(const std::vector<ModelColumn>& columns);
	ModelModel&							addColumn(const ModelColumn& column);
	ModelModel&							setRelations(const std::vector<ModelRelation>& columns);
	ModelModel&							addRelation(const ModelRelation& relation);
	ModelModel&							setResourceColumns(const std::vector<std::string>& resourcedColumns);
	ModelModel&							setSortColumn(const std::string& sortColumn);

private:
	class Data;
	std::shared_ptr<Data>	mData;
};

} // namespace telstra

#endif


