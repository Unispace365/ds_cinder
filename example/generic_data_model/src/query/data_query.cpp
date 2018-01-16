#include "stdafx.h"

#include "data_query.h"

#include <map>
#include <sstream>
#include <ds/debug/logger.h>
#include <ds/query/query_client.h>
#include <ds/util/file_meta_data.h>

#include <ds/app/environment.h>


#include "ds/query/sqlite/sqlite3.h"

namespace downstream {

/**
* \class downstream::DataQuery
*/
DataQuery::DataQuery() {
}

ds::model::DataModelRef DataQuery::readXml() {
	ds::model::DataModelRef output;

	ci::XmlTree xml;

	try {
		xml = ci::XmlTree(cinder::loadFile(ds::Environment::expand("%APP%/data/model/data_model.xml")));
	} catch(ci::XmlTree::Exception &e) {
		DS_LOG_WARNING("loadXmlContent doc not loaded! oh no: " << e.what());
		return output;
	} catch(std::exception &e) {
		DS_LOG_WARNING("loadXmlContent doc not loaded! oh no: " << e.what());
		return output;
	}


	auto rooty = xml.getChild("model");
	int id = 1;
	readXmlNode(rooty, output, id);

	if(output.hasChild("model")) {
		return output.getChild("model");
	}

	return output;
}

void DataQuery::readXmlNode(ci::XmlTree& tree, ds::model::DataModelRef& parentData, int& id) {
	ds::model::DataModelRef thisNode;
	thisNode.setName(tree.getTag());
	thisNode.setId(id++);

	for (auto it : tree.getAttributes()){
		thisNode.setProperty(it.getName(), it.getValue());
	}

	for (auto& it : tree.getChildren()){
		readXmlNode(*it, thisNode, id);
	}

	parentData.addChild(tree.getTag(), thisNode);
}

void DataQuery::run() {

	mData = ds::model::DataModelRef("root");

	const ds::Resource::Id cms(ds::Resource::Id::CMS_TYPE, 0);
	ds::query::Result result;
	ds::query::Result recResult;
	std::string dbPath = cms.getDatabasePath();
	std::string resourcesPath = cms.getResourcePath();

	std::map<int, ds::Resource> allResources;
	//									0			1				2				3				4				5					6			7
	std::string recyQuery = "SELECT resourcesid, resourcestype,resourcesduration,resourceswidth,resourcesheight,resourcesfilename,resourcespath,resourcesthumbid FROM Resources";
	if(ds::query::Client::query(dbPath, recyQuery, recResult)) {
		ds::query::Result::RowIterator	rit(recResult);
		while(rit.hasValue()) {
			ds::Resource reccy;
			int mediaId = rit.getInt(0);
			reccy.setDbId(mediaId);
			reccy.setTypeFromString(rit.getString(1));
			reccy.setDuration(rit.getFloat(2));
			reccy.setWidth(rit.getFloat(3));
			reccy.setHeight(rit.getFloat(4));
			if(reccy.getType() == ds::Resource::WEB_TYPE) {
				reccy.setLocalFilePath(rit.getString(5));
			} else {
				std::stringstream loclPath;
				loclPath << resourcesPath << rit.getString(6) << rit.getString(5);
				reccy.setLocalFilePath(loclPath.str());
			}
			reccy.setThumbnailId(rit.getInt(7));
			allResources[mediaId] = reccy;
			++rit;
		}
	}

	/*
	TODO:
	 - Rework resource cache so it doesn't get reloaded every time
	 - Foreign keys and auto-build children
	 - Display data model visually
	 - Get nested data by string
	*/


	auto metaData = readXml();
	//metaData.printTree(true, "");
	if(metaData.empty()) {
		getDataFromTable(mData, "sqlite_master");
		//auto table = mData.getChild("tables");
		auto tables = mData.getChildren("rows");
		for(auto it : tables) {
			it.setName(it.getProperty("tbl_name").getString());
			getDataFromTable(it, it.getProperty("tbl_name").getString());
		}
	} else {
		getDataFromTable(mData, metaData, dbPath, allResources);
	}
}

void DataQuery::getDataFromTable(ds::model::DataModelRef parentModel, const std::string& theTable) {
	const ds::Resource::Id cms(ds::Resource::Id::CMS_TYPE, 0);

	std::string dbPath = cms.getDatabasePath();
	std::string sampleQuery = "SELECT * FROM " + theTable;
	sqlite3* db = NULL;
	const int sqliteResultCode = sqlite3_open_v2(ds::getNormalizedPath(dbPath).c_str(), &db, SQLITE_OPEN_READONLY, 0);
	if(sqliteResultCode == SQLITE_OK) {
		sqlite3_busy_timeout(db, 1500);

		sqlite3_stmt*		statement;
		const int			err = sqlite3_prepare_v2(db, sampleQuery.c_str(), -1, &statement, 0);
		if(err != SQLITE_OK) {
			sqlite3_finalize(statement);
			DS_LOG_ERROR("SqlDatabase::rawSelect SQL error = " << err << " on select=" << sampleQuery << std::endl);
		} else {
			int id = 1;


			while(true) {
				auto statementResult = sqlite3_step(statement);
				if(statementResult == SQLITE_ROW) {
					auto columnCount = sqlite3_data_count(statement);
					ds::model::DataModelRef thisRow = ds::model::DataModelRef(theTable + "_row", id);
					id++;
					for(int i = 0; i < columnCount; i++) {

						auto columnName = sqlite3_column_name(statement, i);

						/*
						const char * dataType = NULL;
						const char * collSequence = NULL;
						int notNull = 0;
						int primaryKey = 0;
						int autoInc = 0;

						int resulty = sqlite3_table_column_metadata(db, NULL, theTable.c_str(), columnName, &dataType, &collSequence, &notNull, &primaryKey, &autoInc);

						if(dataType) {
							std::cout << " Column " << columnName << " type:" << dataType << " col seq:" << collSequence << " not null:" << notNull << " prim key:" << primaryKey << " autoinc:" << autoInc << std::endl;
						} else {
							std::cout << " Column " << columnName << " type:NULL col seq:" << collSequence << " not null:" << notNull << " prim key:" << primaryKey << " autoinc:" << autoInc << std::endl;
						}
						*/

						auto theText = sqlite3_column_text(statement, i);
						std::string theData = "";
						if(theText) {
							theData = reinterpret_cast<const char*>(theText);
						}
						thisRow.setProperty(columnName, theData);
					}
					parentModel.addChild("rows", thisRow);

				} else {
					sqlite3_finalize(statement);
					break;
				}
			}

			//parentModel.addChild("tables", thisTable);
		}

	} else {
		DS_LOG_ERROR("DataQuery: Unable to access the database " << dbPath << " (SQLite error " << sqliteResultCode << ")." << std::endl);
	}
}

void DataQuery::getDataFromTable(ds::model::DataModelRef parentModel, ds::model::DataModelRef tableDescription, const std::string& dbPath, std::map<int, ds::Resource>& allResources) {

	std::string theTable = tableDescription.getPropertyValue("name");
	ds::model::DataModelRef tableModel;

	if(theTable.empty()) {
		if(tableDescription.getName() != "model") {
			DS_LOG_WARNING("No table name specified in datamodel query");
		}

		tableModel = parentModel;
	} else {

		tableModel.setName(theTable);

		std::string selectStmt = tableDescription.getPropertyValue("select");
		std::string sorting = tableDescription.getPropertyValue("sort");
		std::string whereClause = tableDescription.getPropertyValue("where");
		std::string reccys = tableDescription.getPropertyValue("resources");
		std::string primaryId = tableDescription.getPropertyValue("id");

		// Operations:
		// select + where + sorting
		// query
		// apply resources and primary id (auto id to primary key column if left blank)

		/// Select
		std::stringstream theQuery;
		if(selectStmt.empty()) {
			theQuery << "SELECT * FROM " << theTable;
		} else {
			theQuery << selectStmt;
		}

		/// Where
		if(!whereClause.empty()) {
			theQuery << " WHERE " << whereClause;
		}

		/// Sorting
		if(!sorting.empty()) {
			auto theSorts = ds::split(sorting, ", ", true);

			bool firsty = true;
			for(auto it : theSorts) {
				if(it.empty()) continue;

				if(firsty) {
					theQuery << " ORDER BY ";
				} else {
					theQuery << ", ";
				}

				firsty = false;
				theQuery << it;
			}
		}

		/// Resources
		auto resourceColumns = ds::split(reccys, ", ", true);

		/// Lets do the query!
		sqlite3* db = NULL;
		// open the database
		const int sqliteResultCode = sqlite3_open_v2(ds::getNormalizedPath(dbPath).c_str(), &db, SQLITE_OPEN_READONLY, 0);

		/// if everything went ok
		if(sqliteResultCode == SQLITE_OK) {
			sqlite3_busy_timeout(db, 1500);

			sqlite3_stmt*		statement;
			const int			err = sqlite3_prepare_v2(db, theQuery.str().c_str(), -1, &statement, 0);
			if(err != SQLITE_OK) {
				sqlite3_finalize(statement);
				DS_LOG_ERROR("DataQuery::rawSelect SQL error code=" << err << " message=" << sqlite3_errstr(err) << " on select=" << theQuery.str().c_str() << std::endl);

			} else {

				/// in case there's no id field specified or a primary key column
				int id = 1;

				/// go through all the rows
				while(true) {

					/// if this isn't a row, then we're done with the query
					auto statementResult = sqlite3_step(statement);
					if(statementResult == SQLITE_ROW) {

						bool parsedMetadata = false;

						auto columnCount = sqlite3_data_count(statement);
						ds::model::DataModelRef thisRow = ds::model::DataModelRef(theTable + "_row", id);
						id++;

						for(int i = 0; i < columnCount; i++) {

							auto columnName = sqlite3_column_name(statement, i);

							/// If we don't have a primary id set already, look up the metadata for this column and see if it's the primary key
							if(primaryId.empty() && !parsedMetadata) {
								const char * dataType = NULL;
								const char * collSequence = NULL;
								int notNull = 0;
								int primaryKey = 0;
								int autoInc = 0;
								int resulty = sqlite3_table_column_metadata(db, NULL, theTable.c_str(), columnName, &dataType, &collSequence, &notNull, &primaryKey, &autoInc);

								if(primaryKey) {
									primaryId = columnName;
								}

								/*
								if(mVerbose) {
									if(dataType) {
										std::cout << " Column " << columnName << " type:" << dataType << " col seq:" << collSequence << " not null:" << notNull << " prim key:" << primaryKey << " autoinc:" << autoInc << std::endl;
									} else {
										std::cout << " Column " << columnName << " type:NULL col seq:" << collSequence << " not null:" << notNull << " prim key:" << primaryKey << " autoinc:" << autoInc << std::endl;
									}
								}
								*/
							}

							auto theText = sqlite3_column_text(statement, i);

							std::string theData = "";
							if(theText) {
								theData = reinterpret_cast<const char*>(theText);
							}

							if(columnName == primaryId) {
								thisRow.setId(ds::string_to_int(theData));
							}

							ds::model::DataProperty theProperty(columnName, theData);

							if(std::find(resourceColumns.begin(), resourceColumns.end(), columnName) != resourceColumns.end()) {
								theProperty.setResource(allResources[ds::string_to_int(theData)]);
							}

							thisRow.setProperty(columnName, theProperty);
						}

						// only parse metada for the first row
						parsedMetadata = true;

						tableModel.addChild("rows", thisRow);

					} else {
						sqlite3_finalize(statement);
						break;
					}
				}
			}
		} else {
			DS_LOG_ERROR("DataQuery: Unable to access the database " << dbPath << " (SQLite error " << sqliteResultCode << ")." << std::endl);
		}

		parentModel.addChild(theTable, tableModel);

	} // table name is present

	auto tableChildren = tableDescription.getChildren("table");
	for (auto it : tableChildren){
		getDataFromTable(tableModel, it, dbPath, allResources);
	}
}

} // !namespace downstream

