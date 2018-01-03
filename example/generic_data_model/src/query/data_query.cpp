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
	 - Specify tables to use / ignore
	 - Specify Id column (auto id?)
	 - Foreign keys and auto-build children
	 - Sorting (auto sort on sort_order column)
	 - Multi sorting
	 - Where clauses
	 - Mark columns as resources

	*/


	getDataFromTable(mData, "sqlite_master");
	//auto table = mData.getChild("tables");
	auto tables = mData.getChildren("rows");
	for (auto it : tables){
		it.setName(it.getProperty("tbl_name").getString());
		getDataFromTable(it, it.getProperty("tbl_name").getString());
	}

	/*
	std::string tablesQuery = "SELECT tbl_name FROM sqlite_master";
	if(ds::query::Client::query(dbPath, tablesQuery, result)) {
		ds::query::Result::RowIterator it(result);
		while(it.hasValue()) {
			ds::query::Result tableResult;
			ds::model::DataModelRef thisTable = ds::model::DataModelRef(it.getString(0));
			if(ds::query::Client::query(dbPath, "SELECT * FROM " + thisTable.getName(), tableResult, ds::query::Client::INCLUDE_COLUMN_NAMES_F)) {
				ds::query::Result::RowIterator tit(tableResult);
				while(tit.hasValue()) {

					ds::model::DataModelRef thisRow = ds::model::DataModelRef(thisTable.getName());

					for(int i = 0; i < tableResult.getColumnSize(); i++) {
						std::string theValue = tit.getString(i);
						if(theValue.empty()) {
							if(tableResult.getColumnType(i) == ds::query::QUERY_NUMERIC) {

							}
						}
						thisRow.setProperty(tableResult.getColumnName(i), theValue);
					}

					thisTable.addChild("row", thisRow);

					++tit;
				}
			}
			mData.addChild("table", thisTable);
			++it;
		}
	}
	*/
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
			//ds::model::DataModelRef thisTable = ds::model::DataModelRef(theTable, 0);

			while(true) {
				auto statementResult = sqlite3_step(statement);
				if(statementResult == SQLITE_ROW) {
					auto columnCount = sqlite3_data_count(statement);
					ds::model::DataModelRef thisRow = ds::model::DataModelRef(theTable + "_row", id);
					id++;
					for(int i = 0; i < columnCount; i++) {
						auto columnName = sqlite3_column_name(statement, i);
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

} // !namespace downstream

