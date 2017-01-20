#include "yaml_load_service.h"

#include <map>
#include <sstream>
#include <fstream>

#include <ds/math/random.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/query/query_client.h>
#include <ds/util/string_util.h>

#include <service/model_maker.h>


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

	try{
		std::vector<YAML::Node> nodes = YAML::LoadAll(iff);

		// create a ModelModel for each root node
		for(auto it = nodes.begin(); it < nodes.end(); ++it){
			YAML::Node nodey = (*it);
			if(nodey.Type() == YAML::NodeType::Map){
				for(auto mit = (*it).begin(); mit != (*it).end(); ++mit){
					std::string tableName = (*mit).first.as<std::string>();

					// one of the root tables can be options, and we don't care about it
					if(tableName == "options") continue;

					parseTable(tableName, (*mit).second);
				}
			}
			//parseTable((*it));
		}

	} catch(std::exception ex){
		std::cout << "exception: " << ex.what() << std::endl;
	}


}

void YamlLoadService::parseTable(const std::string& tableName, YAML::Node mainComponentsMap){
	ModelModel mm;

	mm.setTableName(tableName);

	// This holds the top level of tableName, columns, relations, actAs, options
	if(mainComponentsMap.Type() != YAML::NodeType::Map){
		DS_LOG_WARNING("Error reading main components map!");
		return;
	}

	for(auto it = mainComponentsMap.begin(); it != mainComponentsMap.end(); ++it){
		std::string key = (*it).first.as<std::string>();
		YAML::Node mappedNode = (*it).second;

		// We're now loading the table name from the root object
		if(key == "tableName"){
// 			if(mappedNode.Type() == YAML::NodeType::Scalar){
// 				std::string tableName = mappedNode.as<std::string>();
// 				mm.setTableName(tableName);
// 			}

		} else if(key == "customInclude"){
			if(mappedNode.Type() == YAML::NodeType::Scalar){
 				std::string customInclude = mappedNode.as<std::string>();
 				mm.setCustomInclude(customInclude);
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

		} else if(key == "actAs"){
			parseActAs((*it).second, mm);

		} else if(key == "options"){
			// load options
		//	std::cout << "options" << std::endl;

		} else {
			DS_LOG_WARNING("Unknown main component: " << key << " in " << mm.getTableName());
		}
	}

	mOutput.push_back(mm);
}

void YamlLoadService::parseColumn(YAML::Node mappedNode, ModelModel& modelModel){
	std::vector<NodeWithKey> sortedNodes;
	fillSortedVectorForNodeMap(mappedNode, sortedNodes);
	
	// look through the map of columns, now sorted
	for(auto columnIt = sortedNodes.begin(); columnIt != sortedNodes.end(); ++columnIt){

		ModelColumn modelColumn;

		std::string columnName = (*columnIt).key;
		modelColumn.setColumnName(columnName);

		YAML::Node columnProperties = (*columnIt).node;

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

				} else if(columnProperty == "class"){
					modelColumn.setCustomDataType(propertyValueString);
					std::string customEmptyType = propertyValueString;
					std::transform(customEmptyType.begin(), customEmptyType.end(), customEmptyType.begin(), ::toupper);
					customEmptyType = ModelMaker::replaceAllString(customEmptyType, "::", "");
					modelColumn.setCustomEmptyDataName(customEmptyType);

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
	std::vector<NodeWithKey> sortedNodes;
	fillSortedVectorForNodeMap(relationsNode, sortedNodes);

	// look through the map of relations, now sorted
	for(auto relIt = sortedNodes.begin(); relIt != sortedNodes.end(); ++relIt){
		ModelRelation mr;
		mr.setForeignKeyTable((*relIt).key);
		YAML::Node relationMap = (*relIt).node;
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

void YamlLoadService::parseActAs(YAML::Node actAsNode, ModelModel& mm){
	if(actAsNode.Type() != YAML::NodeType::Map){
		DS_LOG_WARNING("Incorrect yaml node type for actAs from: " << mm.getTableName());
		return;
	}
	// look through the map of relations
	for(auto relIt = actAsNode.begin(); relIt != actAsNode.end(); ++relIt){
		std::string actionType = (*relIt).first.as<std::string>();
		if(actionType == "Timestampable"){
			// Timestamps?
		} else if(actionType == "Sortable"){
			// add sort by?
		} else if(actionType == "Resourceable"){
			YAML::Node resourceNode = (*relIt).second;
			if(resourceNode.Type() != YAML::NodeType::Map){
				DS_LOG_WARNING("Problem with resourable actAs in " << mm.getTableName());
				continue;
			}

			for(auto it = resourceNode.begin(); it != resourceNode.end(); ++it){
				std::string propertyType = (*it).first.as<std::string>();

				// I got 99 columns, but an ingoretype or mediatable aint one.
				// Or, the only thing we care about in Resourceable is which columns
				if(propertyType == "columns"){
					YAML::Node resourceColumns = (*it).second;
					if(resourceColumns.Type() != YAML::NodeType::Sequence){
						DS_LOG_WARNING("Trubs reading columns in resourceable actAs in " << mm.getTableName());
						continue;
					}

					std::vector<std::string> columnStrings;
					for(auto cit = resourceColumns.begin(); cit != resourceColumns.end(); ++cit){
						columnStrings.push_back((*cit).as<std::string>());
					}
					mm.setResourceColumns(columnStrings);
				}
			}
		}
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

void YamlLoadService::fillSortedVectorForNodeMap(YAML::Node mappedNode, std::vector<NodeWithKey>& sortedNodes){
	for(auto it = mappedNode.begin(); it != mappedNode.end(); ++it){
		sortedNodes.push_back(NodeWithKey((*it).second, (*it).first.as<std::string>()));
	}

	std::sort(sortedNodes.begin(), sortedNodes.end(), [](const NodeWithKey& a, const NodeWithKey& b){
		return a.key < b.key;
	});
}

} // namespace ds


