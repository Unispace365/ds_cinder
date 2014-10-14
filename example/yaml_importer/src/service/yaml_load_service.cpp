#include "yaml_load_service.h"

#include <map>
#include <sstream>
#include <fstream>

#include <ds/math/random.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/query/query_client.h>
#include <ds/util/string_util.h>


namespace ds {


/* 

This Class loads a yaml file with the following syntax into a vector of ModelModels.
Labels with "Here" at the end are actual values, everything else are keys.

#Generic table - for syntax
TableNameHere
	tableName: tableNameHere
	columns:
		columnNameHere:
			type: typeNameHere
			primary: isPrimaryHere
			autoincrement: autoincrementHere
	relations:
		foreignTableHere:
			local: localColumnNameHere
			foreign: foreignColumnNameHere
			type: typeHere
	actAs:
		actionTypeHere:
			actionOptionsHere: []
	options:
		type: typeHere

#here's what a real table would look like
# Notice in the relations, it's a One-Many relationship between Story-StoryElement. Many story elements per each story. The links go both ways
Story: 
	tableName: story
	columns:
		id:
			type: integer(4)
			unsigned: true 
			#only 1 primary column per table
			primary: true
			autoincrement: true
		title:
			#NOTE: limits (the '(255)') are ignored
			type: string(255)

	relations:
		StoryElement:
			local: id
			foreign: id
			type: many
#NOTE options are ignored
	options:
		type: MYISAM


StoryElement:
	tableName: storyElement
	columns:
		id:
			type: integer(4)
			unsigned: true
			primary: true
			autoincrement: true
		resourceId:
			type: integer(4)
		storyId:
			type: integer(4)
	relations:
		Story:
			local: storyId
			foreign: id
			type: one
	actAs:
		Timestampable:
		Resourceable:
			columns: [resourceId]
			ignoretypes: []
			mediatable: false
	options:
		type: MYISAM


*/

/**
* \class ds::YamlLoadService
*/
YamlLoadService::YamlLoadService() {
	mFileLocation = "";
}

void YamlLoadService::run() {
	mOutput.clear();

	// todo: be able to load multiple files and specific files
	std::ifstream iff(mFileLocation);

	if(!iff){
		DS_LOG_WARNING("Unable to yaml load file: " << mFileLocation);
		return;
	}

	std::vector<YAML::Node> nodes = YAML::LoadAll(iff);
//	for(auto it = nodes.begin(); it < nodes.end(); ++it){
//		printYamlRecursive((*it), 0);
//	}

	// create a ModelModel for each root node
	for(auto it = nodes.begin(); it < nodes.end(); ++it){
		YAML::Node nodey = (*it);
		if(nodey.Type() == YAML::NodeType::Map){
			for(auto mit = (*it).begin(); mit != (*it).end(); ++mit){
				parseTable((*mit).second);
			}
		}
		//parseTable((*it));
	}
}

void YamlLoadService::parseTable(YAML::Node mainComponentsMap){
	ModelModel mm;

	// This holds the top level of tableName, columns, relations, actAs, options
	if(mainComponentsMap.Type() != YAML::NodeType::Map){
		DS_LOG_WARNING("Error reading main components map!");
		return;
	}

	for(auto it = mainComponentsMap.begin(); it != mainComponentsMap.end(); ++it){
		std::string key = (*it).first.as<std::string>();
		YAML::Node mappedNode = (*it).second;
		if(key == "tableName"){
			if(mappedNode.Type() == YAML::NodeType::Scalar){
				std::string tableName = mappedNode.as<std::string>();
				mm.setTableName(tableName);
			}

		} else if(key == "columns"){
			if(mappedNode.Type() == YAML::NodeType::Map){
				parseColumn(mappedNode, mm);
				

			} else {
				DS_LOG_WARNING("Error reading column map!");
				continue;// read next main component
			}

		} else if(key == "relations"){

			parseRelations((*it).second, mm);
			// load relations
		//	std::cout << "Relations" << std::endl;

		} else if(key == "actAs"){
			// load actAs
		//	std::cout << "actAs" << std::endl;

		} else if(key == "options"){
			// load options
		//	std::cout << "options" << std::endl;

		} else {
		//	std::cout << "Unknown main component: " << key << std::endl;
		}
	}

	mOutput.push_back(mm);
}

void YamlLoadService::parseColumn(YAML::Node mappedNode, ModelModel& modelModel){
	// look through the map of columns
	for(auto columnIt = mappedNode.begin(); columnIt != mappedNode.end(); ++columnIt){

		ModelColumn modelColumn;

		std::string columnName = (*columnIt).first.as<std::string>();
		modelColumn.setColumnName(columnName);

		YAML::Node columnProperties = (*columnIt).second;
		if(columnProperties.Type() == YAML::NodeType::Map){

			for(auto it = columnProperties.begin(); it != columnProperties.end(); ++it){
				std::string columnProperty = (*it).first.as<std::string>();
				YAML::Node columnPropertyScalar = (*it).second;

				if(columnPropertyScalar.Type() != YAML::NodeType::Scalar){
					DS_LOG_WARNING("Error reading column property, column: " << columnName << " Property: " << columnProperty);
					continue; // look for the next property
				}

				std::string propertyValueString = columnPropertyScalar.as<std::string>();

				if(columnProperty == "type"){
					modelColumn.setType(ModelColumn::getTypeForString(propertyValueString));

				} else if(columnProperty == "unsigned"){
					modelColumn.setIsUnsigned(parseBool(propertyValueString));

				} else if(columnProperty == "primary"){
					modelColumn.setIsPrimary(parseBool(propertyValueString));

				} else if(columnProperty == "autoincrement"){
					modelColumn.setAutoincrement(parseBool(propertyValueString));

				}
			}

		} else {
			DS_LOG_WARNING("Error reading column properties: " << columnName);
			continue; // go to the next column
		}

		modelModel.addColumn(modelColumn);
	}
}

void YamlLoadService::parseRelations(YAML::Node relationsNode, ModelModel& mm){
	if(relationsNode.Type() != YAML::NodeType::Map){
		DS_LOG_WARNING("Incorrect yaml node type for relations from: " << mm.getTableName());
		return;
	}
	// look through the map of relations
	for(auto relIt = relationsNode.begin(); relIt != relationsNode.end(); ++relIt){
		ModelRelation mr;
		mr.setForeignKeyTable((*relIt).first.as<std::string>());
		YAML::Node relationMap = (*relIt).second;
		if(relationMap.Type() != YAML::NodeType::Map){
			DS_LOG_WARNING("Problem reading relation property maps for relation: " << mr.getForeignKeyTable() << " from: " << mm.getTableName());
			continue;
		}
		for(auto it = relationMap.begin(); it != relationMap.end(); ++it){
			std::string propertyType = (*it).first.as<std::string>();
			if((*it).second.Type() != YAML::NodeType::Scalar){
				DS_LOG_WARNING("Trubs reading relation property: " << propertyType << " on: " << mr.getForeignKeyTable() << " from: " << mm.getTableName());
				continue;
			}
			std::string valueString = (*it).second.as<std::string>();
			if(propertyType == "local"){
				mr.setLocalKeyColumn(valueString);
			} else if(propertyType == "foreign"){
				mr.setForeignKeyColumn(valueString);
			} else if(propertyType == "type"){
				if(valueString == "one"){
					mr.setType(ModelRelation::One);
				} else if(valueString == "many"){
					mr.setType(ModelRelation::Many);
				} else {
					DS_LOG_WARNING("Incorrect value received for relation type on: " << mr.getForeignKeyTable() << " from: " << mm.getTableName());
				}
			} else if(propertyType == "class"){
				mr.setForeignKeyTable(valueString);
			}
		} 

		mm.addRelation(mr);
	}
}

// go through everything and print it out
void YamlLoadService::printYamlRecursive(YAML::Node doc, const int level){
	if(!doc) return;

	for(int i = 0; i < level; i++){
		std::cout << "\t";
	}

	switch(doc.Type()) {

	// Nothing after the name of thing, such as:
	//		MyThing: 
	case YAML::NodeType::Null: // ...
		std::cout << "Null " << level << std::endl;
		break;

	// Just a single thing after the name, such as:
	//		MyThing: TheValue
	case YAML::NodeType::Scalar: // ...
		std::cout << "Scalar: " << doc.as<std::string>() << " " << level << std::endl;
		break;

	// Sort of like an array. Similar to a map, but each thing is not named:
	//		MyThings: [ThingOne, ThingTwo, ThingThree]
	case YAML::NodeType::Sequence: // ...
		for(auto dit = doc.begin(); dit != doc.end(); ++dit) {
			printYamlRecursive((*dit), level + 1);
		}
		//std::cout << "Sequence " << std::endl;
		break;

	// An array but each thing has a key. Each thing can be something else too, like a scalar or other maps:
	//		MyThingOne: Thing
	//		MyThingTwo: [NestedSequence]
	case YAML::NodeType::Map: // ...
		//std::cout << "Map " << std::endl;
		for(auto dit = doc.begin(); dit != doc.end(); ++dit) {
			std::string key, value;
			key = (*dit).first.as<std::string>();
			std::cout << std::endl;
			for(int i = 0; i < level; i++){
				std::cout << "\t";
			}
			std::cout << "Map Key: " << key << " " << level << std::endl;

			YAML::Node node = (*dit).second;
			printYamlRecursive(node, level + 1);
		}
		break;

	// Who knows what this is!
	case YAML::NodeType::Undefined: // ...
		std::cout << "Undefined Entry " << level << std::endl;
		break;
	}
}

bool YamlLoadService::parseBool(const std::string& value){
	if(value.find("t") != std::string::npos){
		return true;
	} else {
		return false;
	}
}

} // namespace ds
