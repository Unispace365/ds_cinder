#include "stdafx.h"

#include "directory_query.h"

#include <Poco/File.h>
#include <Poco/Path.h>
#include <ds/debug/logger.h>
#include <ds/query/query_client.h>
#include <ds/util/string_util.h>
#include <map>
#include <sstream>


#include <ds/app/environment.h>

#include "app/waffles_app_defs.h"

namespace waffles {

DirectoryQuery::DirectoryQuery() {
}

void DirectoryQuery::run() {
	try {
		mOutput = ds::model::ContentModelRef();
		mDrives = ds::model::ContentModelRef("physical_drives");
		query();
	} catch (std::exception const& e) {
		DS_LOG_WARNING("DirectoryQuery exception: " << e.what());
	}
}

void DirectoryQuery::setInputType(const bool getDrives) {
	mListDrives = getDrives;
}

void DirectoryQuery::setDirectoryToLoad(const std::string& directoryPath, const int requestId) {
	mDirectoryPath = directoryPath;
	mRequestId	   = requestId;
}

void DirectoryQuery::query() {

	if (mListDrives) {
		mError = true;

		// first, find the boot volume
		std::wstring boolVolumeName;
		TCHAR		 bootVolume[MAX_PATH + 1] = {0};
		if (GetVolumePathName(L"..", bootVolume, ARRAYSIZE(bootVolume))) {
			boolVolumeName = bootVolume;
		} else {
			DS_LOG_WARNING("DirectoryQuery: Couldn't find the boot volume.");
			return;
		}

		// Then find any additional drives
		std::vector<std::string> roots;
		Poco::Path::listRoots(roots);
		for (auto it : roots) {
			std::wstring rootPath = ds::wstr_from_utf8(it);

			TCHAR volumeName[MAX_PATH + 1]	   = {0};
			TCHAR fileSystemName[MAX_PATH + 1] = {0};
			DWORD serialNumber				   = 0;
			DWORD maxComponentLen			   = 0;
			DWORD fileSystemFlags			   = 0;
			if (GetVolumeInformation(rootPath.c_str(), volumeName, ARRAYSIZE(volumeName), &serialNumber,
									 &maxComponentLen, &fileSystemFlags, fileSystemName, ARRAYSIZE(fileSystemName))) {
				// if we want: GetDriveType will return the type (removeable , fixed, network, cd-rom, etc)
				std::wstring			   volName = volumeName;
				std::wstring			   fsName  = fileSystemName;
				ds::model::ContentModelRef driveMedia;
				driveMedia.setProperty("name", volName);
				driveMedia.setProperty("path", rootPath);

				if (rootPath == boolVolumeName && rootPath.empty()) {
					driveMedia.setProperty("name", std::string("Boot"));
				}
				driveMedia.setProperty("type", std::string("directory"));
				mDrives.addChild(driveMedia);
			}
		}

	} else {

		Poco::File thisFile(mDirectoryPath);
		mError = true;

		bool fileExists = false;
		try {
			if (thisFile.exists()) {
				fileExists = true;
			} else {
				fileExists = false;
			}
		} catch (std::exception& e) {
			mOutput.setProperty("error", true);
			DS_LOG_WARNING("File doesn't exist! " << e.what() << " path: " << mDirectoryPath);
			return;
		}

		if (!fileExists) {
			DS_LOG_WARNING("Directory doesn't exist to get a directory from it! " << mDirectoryPath);
			return;
		}

		if (!thisFile.isDirectory()) {
			DS_LOG_WARNING("Directory query tried to get the contenst of a file (can only use directories)! "
						   << mDirectoryPath);
			return;
		}

		ds::Resource folderIcon	 = ds::Resource(ds::Environment::expand("%APP%/data/images/waffles/icons/1x/Folder_64.png"));
		ds::Resource imageIcon	 = ds::Resource(ds::Environment::expand("%APP%/data/images/waffles/icons/1x/Image_64.png"));
		ds::Resource videoIcon	 = ds::Resource(ds::Environment::expand("%APP%/data/images/waffles/icons/1x/Video_64.png"));
		ds::Resource pdfIcon	 = ds::Resource(ds::Environment::expand("%APP%/data/images/waffles/icons/1x/PDF_64.png"));
		ds::Resource webIcon	 = ds::Resource(ds::Environment::expand("%APP%/data/images/waffles/icons/1x/Link_64.png"));
		ds::Resource invalidIcon = ds::Resource(ds::Environment::expand("%APP%/data/images/waffles/icons/1x/Slide_64.png"));

		static int								mediaId = 1;
		std::vector<ds::model::ContentModelRef> folderMedia;
		std::vector<Poco::File>					files;

		thisFile.list(files);
		for (auto it = files.begin(); it < files.end(); ++it) {

			try {
				Poco::Path	pathy		 = (*it).path();
				std::string pathString	 = pathy.toString();
				std::string theExtension = pathy.getExtension();

				if (theExtension == "sys") continue; // ignoring system files
				if (theExtension == "git") continue; // ignoring git files
				if ((*it).isHidden()) continue;

				auto extensionSize = theExtension.size();
				if (extensionSize > 0) {
					extensionSize++; // remove the period if there was an extension
				}

				int						   thisId = mediaId++;
				ds::model::ContentModelRef thisMedia;
				thisMedia.setProperty("path", pathString);
				thisMedia.setProperty("valid", true);

				if ((*it).isDirectory()) {
					thisMedia.setProperty("type", MEDIA_TYPE_DIRECTORY_LOCAL);
					thisMedia.setPropertyResource("icon", folderIcon);
				} else {
					thisMedia.setProperty("type", MEDIA_TYPE_FILE_LOCAL);
					auto thisResource = ds::Resource(pathString);
					thisMedia.setPropertyResource("media_res", thisResource);
					if (thisResource.getType() == ds::Resource::IMAGE_TYPE) {
						thisMedia.setPropertyResource("icon", imageIcon);
					} else if (thisResource.getType() == ds::Resource::VIDEO_TYPE) {
						thisMedia.setPropertyResource("icon", videoIcon);
					} else if (thisResource.getType() == ds::Resource::WEB_TYPE) {
						thisMedia.setPropertyResource("icon", webIcon);
					} else if (thisResource.getType() == ds::Resource::PDF_TYPE) {
						thisMedia.setPropertyResource("icon", pdfIcon);
					} else {
						thisMedia.setPropertyResource("icon", invalidIcon);
						thisMedia.setProperty("valid", false);
					}
				}


				std::string fileName = pathy.getFileName();
				thisMedia.setProperty("name", std::string(fileName));
				fileName = fileName.substr(0, fileName.length() - extensionSize);
				thisMedia.setProperty("filename", std::string(fileName));
				thisMedia.setProperty("path", std::string(pathString));

				thisMedia.setId(thisId);
				folderMedia.push_back(thisMedia);
			} catch (std::exception& e) {
				DS_LOG_WARNING("exception adding local file " << (*it).path() << " " << e.what());
			}
		}

		std::sort(folderMedia.begin(), folderMedia.end(),
				  [](ds::model::ContentModelRef& a, ds::model::ContentModelRef& b) {
					  std::string upperA = a.getPropertyString("name");
					  std::transform(upperA.begin(), upperA.end(), upperA.begin(), ::toupper);
					  std::string upperB = b.getPropertyString("name");
					  std::transform(upperB.begin(), upperB.end(), upperB.begin(), ::toupper);
					  return upperA < upperB;
				  });

		Poco::Path p = Poco::Path(thisFile.path());
		mOutput.setProperty("type", std::string("file"));
		mOutput.setProperty("path", std::string(thisFile.path()));
		mOutput.setProperty("filename", std::string(p.getFileName()));
		mOutput.setProperty("name", mOutput.getPropertyString("filename"));
		mOutput.setProperty("body", std::string(thisFile.path()));


		mOutput.setChildren(folderMedia);
		mError = false;
	}
}

} // namespace waffles
