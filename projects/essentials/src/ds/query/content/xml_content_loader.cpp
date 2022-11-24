#include "stdafx.h"

#include "xml_content_loader.h"

#include <algorithm>
#include <locale>
#include <map>
#include <sstream>
#include <string>

#include <ds/debug/logger.h>
#include <ds/query/query_client.h>
#include <ds/util/string_util.h>

#include <Poco/File.h>
#include <Poco/Path.h>
#include <cinder/Utilities.h>
#include <cinder/Xml.h>

#include <ds/app/environment.h>

namespace ds { namespace model {

	std::vector<ds::model::GenericContentRef> XmlContentLoader::loadXmlContent(const std::string& xmlPath,
																			   const std::string& mediaBasePath) {

		std::vector<ds::model::GenericContentRef> output;
		if (xmlPath.empty()) {
			DS_LOG_WARNING("Broh, you gotta supply an Xml path!");
			return output;
		}

		if (mediaBasePath.empty()) {
			DS_LOG_WARNING("Come on, set a media base path before loadXmlContent!");
			return output;
		}

		ds::Resource::Id tempId = ds::Resource::Id();
		Poco::File		 mediaDirectory(ds::Environment::expand(mediaBasePath));

		if (!mediaDirectory.exists()) {
			DS_LOG_WARNING("Media path does not exist. Make sure the media base path is valid.");
			return output;
		}

		int contentId = 1;

		ci::XmlTree xml;

		try {
			xml = ci::XmlTree(cinder::loadFile(ds::Environment::expand(xmlPath)));
		} catch (ci::XmlTree::Exception& e) {
			DS_LOG_WARNING("loadXmlContent doc not loaded! oh no: " << e.what());
			return output;
		} catch (std::exception& e) {
			DS_LOG_WARNING("loadXmlContent doc not loaded! oh no: " << e.what());
			return output;
		}

		const std::string  MISSING_IMAGE = "NULL";
		const std::string  defaultFull	 = MISSING_IMAGE;
		const std::string  defaultThumb	 = MISSING_IMAGE;
		std::stringstream  ss;
		const std::string& mediaPath = mediaDirectory.path();

		auto  rooty	  = xml.getChild("contents");
		auto& stories = rooty.getChildren();


		// Look through all children nodes
		for (auto it = stories.begin(); it != stories.end(); ++it) {

			const std::string taggy = (*it)->getTag();
			if (taggy != "content") continue;


			const int thisId = contentId++;

			const std::wstring title	 = ds::wstr_from_utf8((*it)->getAttributeValue<std::string>("title", ""));
			const std::wstring textOne	 = ds::wstr_from_utf8((*it)->getAttributeValue<std::string>("textOne", ""));
			const std::wstring textTwo	 = ds::wstr_from_utf8((*it)->getAttributeValue<std::string>("textTwo", ""));
			const std::wstring textThree = ds::wstr_from_utf8((*it)->getAttributeValue<std::string>("textThree", ""));
			const std::wstring textFour	 = ds::wstr_from_utf8((*it)->getAttributeValue<std::string>("textFour", ""));
			const std::wstring textFive	 = ds::wstr_from_utf8((*it)->getAttributeValue<std::string>("textFive", ""));
			const std::wstring textSix	 = ds::wstr_from_utf8((*it)->getAttributeValue<std::string>("textSix", ""));
			const std::wstring textSeven = ds::wstr_from_utf8((*it)->getAttributeValue<std::string>("textSeven", ""));
			const std::wstring textEight = ds::wstr_from_utf8((*it)->getAttributeValue<std::string>("textEight", ""));
			const std::wstring category	 = ds::wstr_from_utf8((*it)->getAttributeValue<std::string>("category", ""));

			const std::string mediaFile = (*it)->getAttributeValue<std::string>("media", "");

			ss.str("");

			// if http occurs at the beginning, then it's a web type
			if (mediaFile.find("http") != 0) {
				ss << mediaPath << "/";
			}
			ss << mediaFile;

			int			 assetTypeId = ds::Resource::parseTypeFromFilename(ss.str());
			ds::Resource mainMedia	 = ds::Resource(ss.str(), assetTypeId);


			ds::model::GenericContentRef contenty;
			contenty.setTitle(title)
				.setId(thisId)
				.setTextOne(textOne)
				.setTextTwo(textTwo)
				.setTextThree(textThree)
				.setTextFour(textFour)
				.setTextFive(textFive)
				.setTextSix(textSix)
				.setTextSeven(textSeven)
				.setTextEight(textEight)
				.setCategory(category)
				.setMedia(mainMedia);

			output.push_back(contenty);
		}

		return output;
	}
}} // namespace ds::model
