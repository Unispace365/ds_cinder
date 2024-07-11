#pragma once

#include <cinder/Xml.h>
#include <ds/app/event_client.h>
#include <ds/thread/serial_runnable.h>
#include <ds/ui/sprite/sprite_engine.h>

namespace waffles {

/**
 * \class waffles::SettingsService
 *			Save the current settings, for app persistence
 */
class SettingsService {
  public:
	SettingsService(ds::ui::SpriteEngine& eng);

	void initialize();
	void updateBackground();

  private:
	void readMediaItem(ci::XmlTree& theTree, ds::model::ContentModelRef& outputMedia);
	void writeMediaItem(ci::XmlTree& theTree, ds::model::ContentModelRef inputMedia);
	void saveSettings();

	class FileWriteRunnable : public Poco::Runnable {
	  public:
		FileWriteRunnable();
		virtual void run();
		void		 setSaveableXml(const ci::XmlTree& theData);

	  private:
		ci::XmlTree mData;
	};

	std::string							  mSettingsFilePath;
	ds::ui::SpriteEngine&				  mEngine;
	ds::EventClient						  mEventClient;
	ds::SerialRunnable<FileWriteRunnable> mSaveService;
	bool								  mInitialized;
};

} // namespace waffles
