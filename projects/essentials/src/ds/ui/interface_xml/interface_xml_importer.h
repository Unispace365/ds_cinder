#pragma once
#ifndef DS_UI_XML_IMPORT_H_
#define DS_UI_XML_IMPORT_H_

#include <functional>
#include <cinder/Xml.h>
#include <cinder/Color.h>
#include <map>
#include <ds/util/bit_mask.h>
#include <ds/ui/layout/layout_sprite.h>

namespace ds{
namespace ui{

class Sprite;
class SpriteEngine;
class Gradient;

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
	// Name prefix gets applied to the front of all sprite names. 
	// For instance, setting a namePrefix of "button" will make the sprite named "title" in this xml be named "button.title".
	// The name prefix is to support recursive loading of xml's, so you can link one xml to another xml.
	static bool loadXMLto(ds::ui::Sprite * parent, const std::string& xmlFile, NamedSpriteMap &map, SpriteImporter customImporter = nullptr, const std::string& namePrefix = "");
	static bool loadXMLto(ds::ui::Sprite * parent, XmlPreloadData& xmldata, NamedSpriteMap &map, SpriteImporter customImporter = nullptr, const std::string& namePrefix = "");

	// Pre-loads the xml & related css files in preparation for creating sprites later. Removes a lot of the dynamic disk reads associated with importing stuff
	static bool preloadXml(const std::string& xmlFile, XmlPreloadData& outData);

	/// If true, will automatically cache xml interfaces after the first time they're loaded
	static void setAutoCache(const bool doCaching);

	static void setSpriteProperty(ds::ui::Sprite &sprite, ci::XmlTree::Attr &attr, const std::string &referer = "");
	static void setSpriteProperty(ds::ui::Sprite &sprite, const std::string& property, const std::string& value, const std::string &referer = "");

	static std::string getSpriteTypeForSprite(ds::ui::Sprite* sp);

	/// Creates a new sprite based on the type string ("sprite", "text", "layout", etc), 
	/// The value is an optional value, and can provide a default value for some sprite types. For instance, text will get the value as text, image will try to load an image at the value file path
	static ds::ui::Sprite* createSpriteByType(ds::ui::SpriteEngine& engine, const std::string& type, const std::string& value = "");
	static void getSpriteProperties(ds::ui::Sprite& sp, ci::XmlTree& xml);

	// the opposite of loading an xml to a sprite
	static ci::XmlTree createXmlFromSprite(ds::ui::Sprite& sprite);

	static std::string getGradientColorsAsString(ds::ui::Gradient* grad);

	static void dispatchStringEvents(const std::string& str, ds::ui::Sprite* bs, const ci::Vec3f& globalPos);
	static void dispatchSingleEvent(const std::string& str, ds::ui::Sprite* bs, const ci::Vec3f& globalPos);


protected:
	XmlImporter( ds::ui::Sprite *targetSprite, const std::string& xmlFile, NamedSpriteMap &map, SpriteImporter customImporter = nullptr, const std::string& namePrefix = "")
		: mTargetSprite(targetSprite)
		, mXmlFile( xmlFile )
		, mNamedSpriteMap( map )
		, mCustomImporter( customImporter )
		, mNamePrefix(namePrefix)
	{}
	~XmlImporter();

	bool load(ci::XmlTree &);

	bool readSprite(ds::ui::Sprite *, std::unique_ptr<ci::XmlTree>&);

	NamedSpriteMap &			mNamedSpriteMap;
	std::string 				mXmlFile;
	std::string 				mNamePrefix;
	ds::ui::Sprite*				mTargetSprite;
	SpriteImporter				mCustomImporter;
	std::vector< Stylesheet * >	mStylesheets;

	// special map to link sprites together, like scroll bars to scroll areas, entry fields to keyboards, etc
	std::map<ds::ui::Sprite*, std::string> mSpriteLinks;
};

} // namespace ui
} // namespace ds

#endif // DS_UI_XML_IMPORT_H_
