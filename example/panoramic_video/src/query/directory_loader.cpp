#include "directory_loader.h"

#include <map>
#include <sstream>

#include <Poco/File.h>
#include <Poco/Path.h>

#include <ds/debug/logger.h>
#include <ds/query/query_client.h>
#include <ds/util/file_meta_data.h>
#include <ds/app/environment.h>
#include <ds/data/resource.h>


namespace panoramic {

DirectoryLoader::DirectoryLoader() {
}

void DirectoryLoader::run() {
	mOutput.mAllVideos.clear();
	try {
		query(mOutput);
	} catch (std::exception const&) {
	}
}

void DirectoryLoader::query(AllData& output) {
	std::string dirPath = ds::Environment::expand("%DOCUMENTS%/downstream/panoramic_video/");
	if(!ds::safeFileExistsCheck(dirPath)){
		std::cout << "Panoramic videos folder doesn't exist! Make one and add videos at " << dirPath << std::endl;
		return;
	}

	Poco::File thisFile(dirPath);
	if(!thisFile.isDirectory()) return;

	std::vector<Poco::File> files;
	thisFile.list(files);
	for(auto it = files.begin(); it < files.end(); ++it){
		if((*it).isHidden()) continue;

		Poco::Path pathy = (*it).path();
		std::string pathString = pathy.toString();
		
		const int fileType = ds::Resource::parseTypeFromFilename(pathString);
		if(fileType != ds::Resource::VIDEO_TYPE) continue;

		ds::Resource newVideo = ds::Resource(pathString);

		int extensionSize = pathy.getExtension().size();
		if(extensionSize > 0){
			extensionSize++; // remove the period if there was an extension
		}

		std::string fileName = pathy.getFileName();
		fileName = fileName.substr(0, fileName.length() - extensionSize);
		newVideo.setThumbnailFilePath(fileName);

		mOutput.mAllVideos.push_back(newVideo);
	}
}

} // !namespace panoramic


