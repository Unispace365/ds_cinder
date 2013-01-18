#pragma once
#ifndef DS_CONFIG_SETTINGS_H_
#define DS_CONFIG_SETTINGS_H_

#include <map>
#include <vector>
#include <cinder/Color.h>
#include <cinder/Rect.h>
#include "ds/data/resource.h"

namespace ds {

namespace cfg {

/**
 * \class ds::cfg::Settings
 * \brief Store generic settings info.
 * NOTE:  Whether the index is valid or not depends on the data type.  Initially none
 * could have indexes, but I've been adding in that behaviour as needed.
 */
class Settings {
  public:
	  Settings();
    
    // Load the supplied file.  Currently only XML files are supported.
    // If append is true, merge all results into my existing data.  If it's
	  // false, clear me out first (although only clear if the file actually
	  // exists, otherwise leave me alone).
    void							  	readFrom(const std::string&, const bool append = true);

    bool							  	empty() const;
    void							  	clear();

    // This applies to both RGB and RGBA colours, which are always identical.
    int								  	getColorSize(const std::string& name) const;
    int								  	getFloatSize(const std::string& name) const;
    int								  	getIntSize(const std::string& name) const;
    int								  	getRectSize(const std::string& name) const;
    int									  getResourceIdSize(const std::string& name) const;
    int								  	getTextSize(const std::string& name) const;
    int								  	getTextWSize(const std::string& name) const;
    int                   getPointSize(const std::string& name) const;

    // Throw errors if not found
    float								  getFloat(const std::string& name, const int index = 0) const;
    const cinder::Rectf&  getRect(const std::string& name, const int index = 0) const;
    int									  getInt(const std::string& name, const int index = 0) const;
    const Resource::Id&   getResourceId(const std::string& name, const int index = 0) const;
    const ci::Color&      getColor(const std::string& name, const int index = 0) const;
    const ci::ColorA&	    getColorA(const std::string& name, const int index = 0) const;
    const ci::Vec2f&      getSize(const std::string& name, const int index = 0) const;
    const std::string&    getText(const std::string& name, const int index = 0) const;
    const std::wstring&   getTextW(const std::string& name, const int index = 0) const;
    const ci::Vec3f&      getPoint(const std::string& name, const int index = 0) const;
    // Bools are a convenience on text fields that will be either "true" or "false"
    bool								  getBool(const std::string& name, const int index = 0) const;

    // Answer the defaults if not found
    float							  	getFloat(const std::string& name, const int index, const float defaultValue) const;
    cinder::Rectf					getRect(const std::string& name, const int index, const ci::Rectf& defaultValue) const;
    int								  	getInt(const std::string& name, const int index, const int defaultValue) const;
    Resource::Id					getResourceId(const std::string& name, const int index, const Resource::Id& defaultValue) const;
    ci::Color				    	getColor(const std::string& name, const int index, const ci::Color& defaultValue) const;
    ci::ColorA				    getColorA(const std::string& name, const int index, const ci::ColorA& defaultValue) const;
    ci::Vec2f				    	getSize(const std::string& name, const int index, const ci::Vec2f& defaultValue) const;
    std::string						getText(const std::string& name, const int index, const std::string& defaultValue) const;
    std::wstring					getTextW(const std::string& name, const int index, const std::wstring& defaultValue) const;
    const ci::Vec3f&      getPoint(const std::string& name, const int index, const ci::Vec3f& defaultValue) const;
    // Bools are a convenience on text fields that will be either "true" or "false"
    bool							  	getBool(const std::string& name, const int index, const bool defaultValue) const;

    // Iterate over the keys
    void                  forEachTextKey(const std::function<void(const std::string&)>&);

  private:
  	std::map<std::string, std::vector<float>>
                          mFloat;
  	std::map<std::string, std::vector<ci::Rectf>>
                          mRect;
  	std::map<std::string, std::vector<int>>
                          mInt;
  	std::map<std::string, std::vector<Resource::Id>>
                          mRes;
  	std::map<std::string, std::vector<ci::Color>>
                          mColor;
  	std::map<std::string, std::vector<ci::ColorA>>
                          mColorA;
  	std::map<std::string, std::vector<ci::Vec2f>>
                          mSize;
  	std::map<std::string, std::vector<std::string>>
                          mText;
  	std::map<std::string, std::vector<std::wstring>>
                          mTextW;
  	std::map<std::string, std::vector<ci::Vec3f>>
                          mPoints;

    void                  directReadFrom(const std::string& filename, const bool clear);
    void                  directReadXmlFrom(const std::string& filename, const bool clear);

  public:
    class Editor {
      public:
        static const int  SET_MODE = 0;			// Always replace/create the value
        static const int  IF_MISSING_MODE = 1;	// Only create the value if it doesn't exist

      public:
        Editor(Settings&, const int mode = SET_MODE);

        Editor&							clear();

        Editor&							setMode(const int);

        Editor&							setFloat(const std::string& name, const float);
        Editor&							setResourceId(const std::string& name, const Resource::Id&);
        Editor&							setSize(const std::string& name, const ci::Vec2f&);
        Editor&							setText(const std::string& name, const std::string&);
        Editor&							setPoint(const std::string& name, const ci::Vec3f&);

        Editor&							addInt(const std::string& name, const int);
        Editor&							addResourceId(const std::string& name, const Resource::Id&);
        Editor&							addTextW(const std::string& name, const std::wstring&);

      private:
        Settings&						mSettings;
        int							  	mMode;
    };
};

} // namespace cfg

} // namespace ds

#endif // DS_CONFIG_SETTINGS_H_
