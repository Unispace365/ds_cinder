#include "stdafx.h"

#include "content_query.h"

#include <ds/debug/logger.h>
#include <ds/query/query_client.h>
#include <ds/util/file_meta_data.h>
#include <map>
#include <sstream>

#include <ds/app/environment.h>


#include "ds/query/sqlite/sqlite3.h"

namespace ds {

ContentQuery::ContentQuery()
  : mTableId(0) {}

void ContentQuery::run() {
	mData = ds::model::ContentModelRef("sqlite", 0, "The root of all sqlite data");
	mData.setProperty("cms_database", mCmsDatabase);
	mData.setProperty("model_xml", mXmlDataModel);
	mTableId = 0;

	Poco::Timestamp::TimeVal before = Poco::Timestamp().epochMicroseconds();

	auto metaData = readXml();

	mLastUpdatedResource.clear();
	mAllResources.clear();

	auto metaNode = metaData.getChildByName("meta");
	if (!metaNode.empty()){
		// Handle custom db & resource locations from meta node
		auto dbLoc = metaNode.getPropertyString("db_location");
		auto resLoc = metaNode.getPropertyString("resource_location");
		if(!dbLoc.empty() && !resLoc.empty()) {
			mResourceLocation =
				ds::getNormalizedPath(ds::Environment::expand(resLoc));
			try {
				Poco::Path p = Poco::Path(mResourceLocation);
				p.append(dbLoc);
				mCmsDatabase = ds::getNormalizedPath(p);
			} catch (std::exception& e) {
				DS_LOG_WARNING("Exception parsing data model path " << e.what());
				return;
			}

			mData.setProperty("cms_database", mCmsDatabase);
		}
	}

	// Customize resources query / resources table names
	auto resourceNode = metaData.getChildByName("resources");
	if(!resourceNode.empty()){
		for(auto kv : mResourceRemap){
			auto newVal = resourceNode.getPropertyString(kv.first);
			if(!newVal.empty()) mResourceRemap[kv.first] = newVal;
		}

		if(!resourceNode.getPropertyString("check_updated").empty()){
			mCheckUpdatedResources = resourceNode.getPropertyBool("check_updated");

			mData.setProperty("merge_content", !mCheckUpdatedResources);
		}
	}



	if (mCmsDatabase.empty()) {
		DS_LOG_VERBOSE(3, "ContentQuery: no sqlite database location specified.");
		return;
	}

	if (!ds::safeFileExistsCheck(mCmsDatabase, false)) {
		DS_LOG_VERBOSE(1, "ContentQuery: no file found for sqlite database location="
							  << mCmsDatabase << ". Not an issue if you're not using sqlite data.");
		return;
	}

	if (metaNode.empty() || metaNode.getPropertyString("use_resources").empty() ||
		metaNode.getPropertyBool("use_resources")) {
		updateResourceCache();
	}

	if (ds::getLogger().hasVerboseLevel(4)) metaData.printTree(true, "");

	if (metaData.empty()) {
		getDataFromTable(mData, "sqlite_master");
		// auto table = mData.getChild("tables");
		auto tables = mData.getChildren();
		for (auto it : tables) {
			it.setName(it.getProperty("tbl_name").getString());
			getDataFromTable(it, it.getProperty("tbl_name").getString());
		}
	} else {

		/// First we get all the tables independently in a list
		auto tablesData = ds::model::ContentModelRef("tables");
		getDataFromTable(tablesData, metaData, mCmsDatabase, mAllResources, 0, mTableId);

		/// then we link all the tables together based on depth and parent id's
		assembleModels(tablesData);
	}

	Poco::Timestamp::TimeVal after = Poco::Timestamp().epochMicroseconds();

	DS_LOG_VERBOSE(1, "Finished data query in " << (float)(after - before) / 1000000.0f << " seconds.");
}

void ContentQuery::assembleModels(ds::model::ContentModelRef tablesParent) {

	/// find the highest depth
	int maxDepth = 0;
	for (auto it : tablesParent.getChildren()) {
		maxDepth = std::max(maxDepth, it.getPropertyInt("depth"));
	}

	/// work backwards through depth levels assigning children to parents
	for (int i = maxDepth; i > 1; i--) {

		// find all the tables at this depth and apply their rows to the parent rows
		for (auto it : tablesParent.getChildren()) {
			if (it.getPropertyInt("depth") == i) {

				// find the parent model for this table
				ds::model::ContentModelRef parentModel = tablesParent.getChildById(it.getPropertyInt("parent_id"));
				if (parentModel.empty()) {
					DS_LOG_WARNING("ContentQuery::assembleModels() no parent table found! this will leave the table "
								   << it.getName() << " orphaned!");
					continue;
				}

				auto childLocalId	= it.getPropertyString("child_local_id");
				auto parentForeignId = it.getPropertyString("parent_foreign_id");
				auto childLocalMap   = it.getPropertyString("child_local_map");

				std::function<bool(ds::model::ContentModelRef&, ds::model::ContentModelRef&)> isMatchFn;

				if (!childLocalId.empty()) {
					isMatchFn = [childLocalId](ds::model::ContentModelRef& parChild, ds::model::ContentModelRef& row) {
						return parChild.getId() == row.getPropertyInt(childLocalId);
					};
				} else if (!parentForeignId.empty()) {
					isMatchFn = [parentForeignId](ds::model::ContentModelRef& parChild,
												   ds::model::ContentModelRef& row) {
						return parChild.getPropertyInt(parentForeignId) == row.getId();
					};
				} else if (!childLocalMap.empty()) {
					auto mapChildTo = ds::split(childLocalMap, ":", true);
					if (mapChildTo.size() == 2) {
						isMatchFn = [mapChildTo](ds::model::ContentModelRef& parChild,
												  ds::model::ContentModelRef& row) {
							return parChild.getPropertyString(mapChildTo[1]) == row.getPropertyString(mapChildTo[0]);
						};
					} else {
						DS_LOG_WARNING("ContentQuery::assembleModels() child_local_map parameter invalid.");
						// Nothing to do here!
						continue;
					}
				}

				for (auto row : it.getChildren()) {
					for (auto parChild : parentModel.getChildren()) {
						if (isMatchFn(parChild, row)) {
							parChild.addChild(row);
						}
					}
				} // End of this table's rows
			} // End of this depth check
		} // End of tables in this for loop
	} // End of depth for loop

	/// assign top level to the final output
	for (auto it : tablesParent.getChildren()) {
		if (it.getPropertyInt("depth") == 1) {
			mData.addChild(it);
		}
	}
}

ds::model::ContentModelRef ContentQuery::readXml() {
	ds::model::ContentModelRef output;

	auto filePath = ds::Environment::expand(mXmlDataModel);
	if (!ds::safeFileExistsCheck(filePath, false)) {
		DS_LOG_VERBOSE(1, "ContentQuery: xml data model file not found.");
		return output;
	}

	ci::XmlTree xml;

	try {
		xml = ci::XmlTree(cinder::loadFile(filePath));
	} catch (ci::XmlTree::Exception& e) {
		DS_LOG_WARNING("ContentQuery readXml() doc not loaded! " << e.what());
		return output;
	} catch (std::exception& e) {
		DS_LOG_WARNING("ContentQuery readXml() doc not loaded! oh no: " << e.what());
		return output;
	}

	auto rooty = xml.getChild("model");
	int  id	= 1;
	readXmlNode(rooty, output, id);

	if (output.hasDirectChild("model")) {
		return output.getChildByName("model");
	}

	return output;
}

void ContentQuery::readXmlNode(ci::XmlTree& tree, ds::model::ContentModelRef& parentData, int& id) {
	ds::model::ContentModelRef thisNode;
	thisNode.setName(tree.getTag());
	thisNode.setId(id++);

	for (auto it : tree.getAttributes()) {
		thisNode.setProperty(it.getName(), it.getValue());
	}

	for (auto& it : tree.getChildren()) {
		readXmlNode(*it, thisNode, id);
	}

	parentData.addChild(thisNode);
}

std::string getSqliteString(sqlite3_stmt* statement, const int columnIndex) {
	auto		theText = sqlite3_column_text(statement, columnIndex);
	std::string theData = "";
	if (theText) {
		theData = reinterpret_cast<const char*>(theText);
	}
	return theData;
}

/// TODO: rewrite to use raw sqlite calls or use column names for better portability
/// TODO: Improve speed
void ContentQuery::updateResourceCache() {

	DS_LOG_VERBOSE(1, "ContentQuery: updateResourceCache");
	ds::query::Result recResult;

	auto resQuery = std::string("SELECT");
	resQuery.append(" " + mResourceRemap["id"]);			   // 0
		resQuery.append(", " + mResourceRemap["type"]);		   // 1
		resQuery.append(", " + mResourceRemap["duration"]);	   // 2
		resQuery.append(", " + mResourceRemap["width"]);	   // 3
		resQuery.append(", " + mResourceRemap["height"]);	   // 4
		resQuery.append(", " + mResourceRemap["filename"]);	   // 5
		resQuery.append(", " + mResourceRemap["path"]);		   // 6
		resQuery.append(", " + mResourceRemap["thumb"]);	   // 7

	if(mCheckUpdatedResources){
		resQuery.append(", " + mResourceRemap["updated"]);	   // 8
	}

	resQuery.append(" FROM " + mResourceRemap["table_name"] + " ");

	if (mCheckUpdatedResources && !mLastUpdatedResource.empty()) {
		resQuery.append("WHERE " + mResourceRemap["updated"] + " > '");
		resQuery.append(mLastUpdatedResource);
		resQuery.append("' ");
	}

	if(mCheckUpdatedResources){
		resQuery.append("ORDER BY "+ mResourceRemap["updated"] +" ASC");
	}

	/// Lets do the query!
	sqlite3* db = NULL;
	// open the database
	const int sqliteResultCode =
		sqlite3_open_v2(ds::getNormalizedPath(mCmsDatabase).c_str(), &db, SQLITE_OPEN_READONLY, 0);

	/// if everything went ok
	if (sqliteResultCode == SQLITE_OK) {
		sqlite3_busy_timeout(db, 1500);
		sqlite3_stmt* statement;
		const int	 err = sqlite3_prepare_v2(db, resQuery.c_str(), -1, &statement, 0);
		if (err != SQLITE_OK) {
			sqlite3_finalize(statement);
			DS_LOG_ERROR("ContentQuery::updateResourceQuery::rawSelect SQL error code="
					<< err << " message=" << sqlite3_errstr(err) << " on select=" << resQuery << std::endl);

		} else {

			/// go through all the rows
			while (true) {

				/// if this isn't a row, then we're done with the query
				auto statementResult = sqlite3_step(statement);
				if (statementResult == SQLITE_ROW) {

					int			thisId  = sqlite3_column_int(statement, 0);
					std::string thePath = getSqliteString(statement, 6);

					mAllResources[thisId] = ds::Resource(thisId,  // db id
							ds::Resource::makeTypeFromString(getSqliteString(
									statement, 1)),  // type (image, video, pdf) as int
							sqlite3_column_double(statement, 2),		  // duration
							(float)sqlite3_column_double(statement, 3),  // width
							(float)sqlite3_column_double(statement, 4),  // height
							getSqliteString(statement, 5),				  // filename
							thePath,									  // path
							sqlite3_column_int(statement, 7),			  // thumbnail id
							""  // full filepath (set in a second)
							);

					auto& reccy = mAllResources[thisId];
					if (reccy.getType() == ds::Resource::WEB_TYPE) {
						auto webPath = reccy.getFileName();
						// detect if this is a local path
						if (webPath.find("http") != 0 && webPath.find("ftp") != 0) {
							std::stringstream loclPath;
							loclPath << mResourceLocation << thePath << webPath;
							webPath = loclPath.str();
							std::replace(webPath.begin(), webPath.end(), '\\', '/');
						}
						reccy.setLocalFilePath(webPath, false);
					} else {
						std::stringstream loclPath;
						loclPath << mResourceLocation << thePath << reccy.getFileName();
						std::string ret = loclPath.str();
						std::replace(ret.begin(), ret.end(), '\\', '/');
						reccy.setLocalFilePath(ret, false);
					}

					if(mCheckUpdatedResources){
						mLastUpdatedResource = getSqliteString(statement, 8);
					}

				} else {
					sqlite3_finalize(statement);
					break;
				}
			}
		}
	} else {
		DS_LOG_ERROR("ContentQuery:updateResourceQuery Unable to access the database "
				<< mCmsDatabase << " (SQLite error " << sqliteResultCode << ")." << std::endl);
	}

	DS_LOG_VERBOSE(1, "ContentQuery: updateResourceCache lastUpdated=" << mLastUpdatedResource);
}

void ContentQuery::getDataFromTable(ds::model::ContentModelRef parentModel, ds::model::ContentModelRef tableDescription,
									const std::string& dbPath, std::unordered_map<int, ds::Resource>& allResources,
									const int depth, const int parentModelId) {

	std::string theTable	  = tableDescription.getPropertyValue("table_name");
	std::string theTableAlias = tableDescription.getPropertyValue("name");

	if (theTable.empty() && !theTableAlias.empty()) {
		// If only "name" is provided, use it as both the alias and SQL table name
		theTable = theTableAlias;
	} else if (!theTable.empty() && theTableAlias.empty()) {
		// If for some reason the user only provieds "table_name", use it for both as well
		theTableAlias = theTable;
	}

	int						   thisId	 = mTableId++;
	ds::model::ContentModelRef tableModel = ds::model::ContentModelRef(theTableAlias, thisId, "SQLite Table");

	if (theTable.empty()) {
		if (tableDescription.getName() != "model" && tableDescription.getName() != "meta" && tableDescription.getName() != "resources") {
			DS_LOG_WARNING("ContentQuery::getDataFromTable() No table name specified in datamodel query");
		}

		tableModel = parentModel;

	} else {

		std::string selectStmt  = tableDescription.getPropertyString("select");
		std::string sorting		= tableDescription.getPropertyString("sort");
		std::string whereClause = tableDescription.getPropertyString("where");
		std::string limits		= tableDescription.getPropertyString("limit");
		std::string reccys		= tableDescription.getPropertyString("resources");
		std::string primaryId   = tableDescription.getPropertyString("id");
		std::string theName		= tableDescription.getPropertyString("name_field");
		std::string theLabel	= tableDescription.getPropertyString("label_field");

		tableModel.setProperties(tableDescription.getProperties());
		tableModel.setProperty("depth", depth);
		tableModel.setProperty("parent_id", parentModelId);
		tableModel.setId(thisId);


		/// Select
		std::stringstream theQuery;
		if (selectStmt.empty()) {
			theQuery << "SELECT * FROM " << theTable;
		} else {
			theQuery << selectStmt;
		}

		/// Where
		if (!whereClause.empty()) {
			theQuery << " WHERE " << whereClause;
		}

		/// Sorting
		if (!sorting.empty()) {
			auto theSorts = ds::split(sorting, ", ", true);

			bool firsty = true;
			for (auto it : theSorts) {
				if (it.empty()) continue;

				if (firsty) {
					theQuery << " ORDER BY ";
				} else {
					theQuery << ", ";
				}

				firsty = false;
				theQuery << it;
			}
		}

		if (!limits.empty()) {
			theQuery << " LIMIT " << limits;
		}

		/// Resources
		auto resourceColumns = ds::split(reccys, ", ", true);

		/// Lets do the query!
		sqlite3* db = NULL;
		// open the database
		const int sqliteResultCode =
			sqlite3_open_v2(ds::getNormalizedPath(dbPath).c_str(), &db, SQLITE_OPEN_READONLY, 0);

		/// if everything went ok
		if (sqliteResultCode == SQLITE_OK) {
			sqlite3_busy_timeout(db, 1500);

			DS_LOG_VERBOSE(4, "Executing SQL query " << theQuery.str());

			sqlite3_stmt* statement;
			const int	 err = sqlite3_prepare_v2(db, theQuery.str().c_str(), -1, &statement, 0);
			if (err != SQLITE_OK) {
				sqlite3_finalize(statement);
				DS_LOG_ERROR("ContentQuery::rawSelect SQL error code=" << err << " message=" << sqlite3_errstr(err)
																	   << " on select=" << theQuery.str().c_str()
																	   << std::endl);

			} else {

				/// in case there's no id field specified or a primary key column
				int id = 1;

				/// go through all the rows
				while (true) {

					/// if this isn't a row, then we're done with the query
					auto statementResult = sqlite3_step(statement);
					if (statementResult == SQLITE_ROW) {

						bool parsedMetadata = false;

						auto					   columnCount = sqlite3_data_count(statement);
						ds::model::ContentModelRef thisRow =
							ds::model::ContentModelRef(theTableAlias, id, theTable + " row");
						id++;

						for (int i = 0; i < columnCount; i++) {

							auto columnName = sqlite3_column_name(statement, i);

							/// If we don't have a primary id set already, look up the metadata for this column and see
							/// if it's the primary key
							if (primaryId.empty() && !parsedMetadata) {
								const char* dataType	 = NULL;
								const char* collSequence = NULL;
								int			notNull		 = 0;
								int			primaryKey   = 0;
								int			autoInc		 = 0;
								int			resulty =
									sqlite3_table_column_metadata(db, NULL, theTable.c_str(), columnName, &dataType,
																  &collSequence, &notNull, &primaryKey, &autoInc);

								if (primaryKey) {
									primaryId = columnName;
								}

								if (ds::getLogger().hasVerboseLevel(3)) {
									if (dataType) {
										DS_LOG_VERBOSE(3, " Column "
															  << columnName << " type:" << dataType
															  << " col seq:" << collSequence << " not null:" << notNull
															  << " prim key:" << primaryKey << " autoinc:" << autoInc);
									} else {
										DS_LOG_VERBOSE(3, " Column "
															  << columnName << " type:NULL col seq:" << collSequence
															  << " not null:" << notNull << " prim key:" << primaryKey
															  << " autoinc:" << autoInc);
									}
								}
							}

							auto theText = sqlite3_column_text(statement, i);

							auto theInt  = sqlite3_column_int(statement, i);
							auto theDoub = sqlite3_column_double(statement, i);

							std::string theData = "";
							if (theText) {
								theData = reinterpret_cast<const char*>(theText);
							}

							if (columnName == primaryId) {
								thisRow.setId(theInt);
							}

							if (!theName.empty() && columnName == theName) {
								thisRow.setName(theData);
							}
							if (!theLabel.empty() && columnName == theLabel) {
								thisRow.setLabel(theData);
							}

							thisRow.setProperty(columnName,
												ds::model::ContentProperty(columnName, theData, theInt, theDoub));

							if (!resourceColumns.empty() && std::find(resourceColumns.begin(), resourceColumns.end(),
																	  columnName) != resourceColumns.end()) {
								thisRow.setPropertyResource(columnName, allResources[ds::string_to_int(theData)]);
							}
						}

						// only parse metada for the first row
						parsedMetadata = true;

						tableModel.addChild(thisRow);


					} else {
						sqlite3_finalize(statement);
						break;
					}
				}
			}
		} else {
			DS_LOG_ERROR("ContentQuery: Unable to access the database " << dbPath << " (SQLite error "
																		<< sqliteResultCode << ")." << std::endl);
		}

		parentModel.addChild(tableModel);

	}  // table name is present

	auto tableChildren = tableDescription.getChildren();
	for (auto it : tableChildren) {
		getDataFromTable(parentModel, it, dbPath, allResources, depth + 1, thisId);
	}
}

void ContentQuery::getDataFromTable(ds::model::ContentModelRef parentModel, const std::string& theTable) {
	const ds::Resource::Id cms(ds::Resource::Id::CMS_TYPE, 0);

	std::string dbPath			 = cms.getDatabasePath();
	std::string sampleQuery		 = "SELECT * FROM " + theTable;
	sqlite3*	db				 = NULL;
	const int   sqliteResultCode = sqlite3_open_v2(ds::getNormalizedPath(dbPath).c_str(), &db, SQLITE_OPEN_READONLY, 0);
	if (sqliteResultCode == SQLITE_OK) {
		sqlite3_busy_timeout(db, 1500);

		sqlite3_stmt* statement;
		const int	 err = sqlite3_prepare_v2(db, sampleQuery.c_str(), -1, &statement, 0);
		if (err != SQLITE_OK) {
			sqlite3_finalize(statement);
			DS_LOG_ERROR("SqlDatabase::rawSelect SQL error = " << err << " on select=" << sampleQuery << std::endl);
		} else {
			int id = 1;


			while (true) {
				auto statementResult = sqlite3_step(statement);
				if (statementResult == SQLITE_ROW) {
					auto					   columnCount = sqlite3_data_count(statement);
					ds::model::ContentModelRef thisRow	 = ds::model::ContentModelRef(theTable + "_row", id);
					id++;
					for (int i = 0; i < columnCount; i++) {

						auto columnName = sqlite3_column_name(statement, i);

						auto		theText = sqlite3_column_text(statement, i);
						std::string theData = "";
						if (theText) {
							theData = reinterpret_cast<const char*>(theText);
						}
						thisRow.setProperty(columnName, theData);
					}
					parentModel.addChild(thisRow);

				} else {
					sqlite3_finalize(statement);
					break;
				}
			}

			// parentModel.addChild("tables", thisTable);
		}

	} else {
		DS_LOG_ERROR("ContentQuery: Unable to access the database " << dbPath << " (SQLite error " << sqliteResultCode
																	<< ")." << std::endl);
	}
}

}  // namespace ds
