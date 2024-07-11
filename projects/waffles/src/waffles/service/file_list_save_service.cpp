#include "stdafx.h"

#include "file_list_save_service.h"

#include <Poco/File.h>
#include <Poco/Path.h>

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/query/query_client.h>
#include <ds/util/string_util.h>

#include "app/app_defs.h"

namespace waffles {

FileListSaveService::FileListSaveService() {
}

void FileListSaveService::run() {
	mFileList.write(ci::writeFile(ds::Environment::expand("%LOCAL%/waffles-neu/recent_files.xml")));
}

void FileListSaveService::setFileList(const ci::XmlTree& fileList) {
	mFileList = fileList;
}

} // namespace waffles
