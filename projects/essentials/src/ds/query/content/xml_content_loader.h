#ifndef DS_QUERY_CONTENT_XML_CONTENT_LOADER
#define DS_QUERY_CONTENT_XML_CONTENT_LOADER

#include <Poco/Runnable.h>
#include <ds/query/query_result.h>
#include <functional>

#include "generic_content_model.h"

namespace ds { namespace model {

	/* A helper class to get some dummy content into your app quickly without having to modify a sqlite file or
	 * hard-coding assets. */
	class XmlContentLoader {
	  public:
		XmlContentLoader(){};

		// xmlPath: the path to the xml that defines the content. Auto-expanded to the environment. Example:
		// %APP%/data/temp/story_content.xml mediaBasePath: Where your media files are located (if any). Paths to media
		// will append to this. Media base path is requied, even if you don't have any media
		static std::vector<ds::model::GenericContentRef> loadXmlContent(const std::string& xmlPath,
																		const std::string& mediaBasePath);


		// example xml:
		/*
		<contents>
			<content title="The emergence of technology in a technological era" textOne="Art thing" media="art.png"
		category="Emerging Technologies" />
		</contents>

		// Supply as many "content" entries as you want
		// There are a lot of text fields which could be converted to other types after loading, if you want (for
		instance, ds::string_to_value<float>() or whatnot
		// attributes: title, category, media, textOne, textTwo, textThree, textFour, textFive, textSix, textSeven,
		textEight
		*/
	};

}} // namespace ds::model

#endif