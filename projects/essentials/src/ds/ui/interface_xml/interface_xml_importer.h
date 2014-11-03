#pragma once
#ifndef DS_UI_XML_IMPORT_H_
#define DS_UI_XML_IMPORT_H_

#include <functional>
#include <cinder/Xml.h>
#include <map>

namespace ds{
namespace ui{

class Sprite;
struct Stylesheet;

class XmlImporter {

public:
	struct XmlPreloadData {
		ci::XmlTree					mXmlTree;
		std::vector<Stylesheet*>	mStylesheets;
		std::string					mFilename;
	};

	typedef std::function< ds::ui::Sprite*(const std::string &typeName, ci::XmlTree &) > SpriteImporter;
	typedef std::map< std::string, ds::ui::Sprite *> NamedSpriteMap;

	// reads the xml at the specified path and creates any sprites found on the parent
	// Returns true if the xml was loaded and sprites were successfully created
	// False indicates either xml load failure or failure to create sprites
	static bool loadXMLto(ds::ui::Sprite * parent, const std::string& xmlFile, NamedSpriteMap &map, SpriteImporter customImporter = nullptr);
	static bool loadXMLto(ds::ui::Sprite * parent, XmlPreloadData& xmldata, NamedSpriteMap &map, SpriteImporter customImporter = nullptr);

	// Pre-loads the xml & related css files in preparation for creating sprites later. Removes a lot of the dynamic disk reads associated with importing stuff
	static bool preloadXml(const std::string& xmlFile, XmlPreloadData& outData);

protected:
	XmlImporter( ds::ui::Sprite *targetSprite, const std::string& xmlFile, NamedSpriteMap &map, SpriteImporter customImporter = nullptr)
		: mTargetSprite(targetSprite)
		, mXmlFile( xmlFile )
		, mNamedSpriteMap( map )
		, mCustomImporter( customImporter )
	{}
	~XmlImporter();

	bool load(ci::XmlTree &);

	bool readSprite(ds::ui::Sprite *, std::unique_ptr<ci::XmlTree>& );

	NamedSpriteMap &			mNamedSpriteMap;
	std::string 				mXmlFile;
	ds::ui::Sprite *			mTargetSprite;
	SpriteImporter				mCustomImporter;
	std::vector< Stylesheet * >	mStylesheets;
};

} // namespace ui
} // namespace ds

#endif // DS_UI_XML_IMPORT_H_
