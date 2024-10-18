#pragma once

#include <optional>

#include <ds/ui/layout/smart_layout.h>
#include <ds/ui/sprite/sprite_engine.h>
namespace waffles {
class TemplateBase;

// A TemplateDef defines the mapping between human readable template name, CMS ID of the content type, and a layout XML
struct TemplateDef {
	std::string name;	   // Human readable name
	std::string id;		   // Bridge UUID of the schema type
	std::string layoutXml; // XML layout for template
	std::vector<std::string> extra;
	bool		requiresClear = false;
};

typedef std::tuple<std::string, std::string, std::string, std::string> TemplateDefTuple;

class TemplateConfig {
public:
	TemplateConfig(ds::ui::SpriteEngine& eng);

	template<class T>
	static void initDefault(ds::ui::SpriteEngine& eng) {
		auto config = TemplateConfig::getDefault<T>(&eng);
	}

	template<class T=TemplateConfig>
	static TemplateConfig* getDefault(ds::ui::SpriteEngine* e=nullptr) {
		static TemplateConfig* sDefault = new T(*e);
		return sDefault;
	}
	virtual const TemplateDef& getTemplateDefFromName(const std::string& name);
	virtual const TemplateDef& getTemplateDefFromId(const std::string& id);
	virtual TemplateBase*	   createTemplate(ds::ui::SpriteEngine& engine, 
										      ds::model::ContentModelRef model,
											  std::string channel_name = "");
	virtual void initializeTemplateDefs();
private:
	
	std::vector<TemplateDef> mTemplateDefinitions;
	ds::ui::SpriteEngine& mEngine;
};

/*
namespace config {
	// All the valid templates for the application
	const std::vector<TemplateDef> TemplateDefinitions = {
		{"invalid",				"FiddleSticks", "template/invalid.xml",				false},
		{"media",				  "ykAdRYGJeuXI", "template/media.xml",				false},
		{"particle_message",	 "ScCWZMV8UNbx", "template/particle_text.xml",	   true },
		{"triangle_message",	 "DvjPISrKtmYf", "template/triangle_message.xml",	  true },
		{"composite_slide",		"Pj63JiWllbSY", "template/composite_slide.xml",		true },
		{"empty",				  "EMPTY",		   "template/empty.xml",				 true },
		{"particle_background", "WoooWoooWooo", "template/particle_background.xml", false},
		{"media_gallery",		  "MqqxszXodMT0", "template/media_gallery.xml",		true },
		{"bubbles",				"HvVljKx8LsLB", "template/bubbles.xml",				true },
		{"cards",				  "AiqfXIk1LW9M", "template/cards.xml",				true },
		{"carousel_cards",	   "7a3Eee0B1G7y", "template/carousel_cards.xml",	  true },
		{"custom_layout",		  "Pj63JiWllbSY", "template/custom_layout.xml",		true },
		{"feature_story",		  "FRLQL8ipKv66", "template/feature_story.xml",		true },
		{"pinboard_event",	   "wVSdlIunhl7D", "template/pinboard.xml",			true },
		{"assets_mode",			"Assets Thing", "template/empty.xml",				  true },
		{"codice_object",		  "ntmSB9wkQDjc", "template/dial_overview.xml",		true },
	};

} // namespace Config

/// Get the template definition for template called 'name'
const TemplateDef& getTemplateDefFromName(const std::string& name);

/// Get the template definition for template with id == 'id'
const TemplateDef& getTemplateDefFromId(const std::string& id);

TemplateBase* createTemplate(ds::ui::SpriteEngine& engine, ds::model::ContentModelRef model);
*/
} // namespace waffles
