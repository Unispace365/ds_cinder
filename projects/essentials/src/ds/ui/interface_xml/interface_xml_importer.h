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
	static bool loadXMLto(ds::ui::Sprite * parent, const std::string& xmlFile, NamedSpriteMap &map, SpriteImporter customImporter = nullptr);
	static bool loadXMLto(ds::ui::Sprite * parent, XmlPreloadData& xmldata, NamedSpriteMap &map, SpriteImporter customImporter = nullptr);

	// Pre-loads the xml & related css files in preparation for creating sprites later. Removes a lot of the dynamic disk reads associated with importing stuff
	static bool preloadXml(const std::string& xmlFile, XmlPreloadData& outData);

	/// If true, will automatically cache xml interfaces after the first time they're loaded
	static void setAutoCache(const bool doCaching);

	static void setSpriteProperty(ds::ui::Sprite &sprite, ci::XmlTree::Attr &attr, const std::string &referer = "");
	static void setSpriteProperty(ds::ui::Sprite &sprite, const std::string& property, const std::string& value, const std::string &referer = "");

	static std::string RGBToHex(ci::Color color);
	static std::string RGBToHex(int rNum, int gNum, int bNum);

	// Straight-up string to ci::ColorA conversion
	static ci::ColorA parseHexColor(const std::string &color);
	// Checks engine named colors, then does a string convert if it's not found
	static ci::ColorA parseColor(const std::string &color, const ds::ui::SpriteEngine& engine);

	static const ds::BitMask parseMultitouchMode(const std::string& s);
	static const std::string getMultitouchStringForBitMask(const ds::BitMask& mask);

	static std::string getSpriteTypeForSprite(ds::ui::Sprite* sp);

	/// Creates a new sprite based on the type string ("sprite", "text", "layout", etc), 
	/// The value is an optional value, and can provide a default value for some sprite types. For instance, text will get the value as text, image will try to load an image at the value file path
	static ds::ui::Sprite* createSpriteByType(ds::ui::SpriteEngine& engine, const std::string& type, const std::string& value = "");
	static void getSpriteProperties(ds::ui::Sprite& sp, ci::XmlTree& xml);

	// the opposite of loading an xml to a sprite
	static ci::XmlTree createXmlFromSprite(ds::ui::Sprite& sprite);

	static std::string getLayoutSizeModeString(const int sizeMode);
	static std::string getLayoutVAlignString(const int vAlign);
	static std::string getLayoutHAlignString(const int vAlign);
	static std::string getLayoutTypeString(const ds::ui::LayoutSprite::LayoutType& propertyValue);
	static std::string getShrinkToChildrenString(const ds::ui::LayoutSprite::ShrinkType& propertyValue);

	static std::string getGradientColorsAsString(ds::ui::Gradient* grad);


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
