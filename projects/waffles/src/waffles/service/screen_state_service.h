#pragma once

#include <cinder/Xml.h>
#include <ds/app/auto_update.h>
#include <ds/app/event_client.h>
#include <ds/thread/serial_runnable.h>

namespace waffles {
class BaseElement;

/**
 * \class waffles::ScreenStateService
 *			Save and retrieve what panels are onscreen and where
 */
class ScreenStateService : public ds::AutoUpdate {
  public:
	ScreenStateService(ds::ui::SpriteEngine&);


	/// First run reading of files and stuff
	void initialize();

	/// Reads the save state xml and puts the metadata into allData
	void parseStateMetadata();

	/// Periodically save the current state of the app to a file
	void autoSaveState();

	/// Restore the auto save state
	void restoreAutoSaveState();

	/// Saves the current state of the app as the named state (if this name already exists, replaces it)
	void saveNamedState(const std::string& stateName);

	/// Loads the named state and closes everything else in the app (fails if the named state doesn't exist, returns
	/// false)
	bool loadNamedState(const std::string& stateName);

	/// Deletes a named state
	void deleteNamedState(const std::string& stateName);

	/// If a state with this name exists already
	bool hasNamedState(const std::string& stateName);

	/// Saves the current state of the app into this xml tree (doesn't actually save a file or anything)
	void saveState(ci::XmlTree& tree);
	/// closes everything in the app and loads everything in this tree (assumes a list of "viewer" tags)
	void loadState(ci::XmlTree& tree);

  private:
	virtual void update(const ds::UpdateParams&) override;
	void		 readMediaItem(ci::XmlTree& theTree);
	void		 writeViewer(ci::XmlTree& theTree, BaseElement* bp);

	class FileWriteRunnable : public Poco::Runnable {
	  public:
		FileWriteRunnable();
		virtual void run();
		void		 setSaveableXml(const ci::XmlTree& theData, std::string saveFileName);
		std::string	 mSaveFileName;

	  private:
		ci::XmlTree mData;
	};

	bool					 mInitialized;
	bool					 mAutoSave;
	int						 mAutoSaveDuration;
	Poco::Timestamp::TimeVal mLastAutoSave;
	ci::XmlTree				 mSaveStateXml;

	ds::ui::SpriteEngine&				  mEngine;
	ds::EventClient						  mEventClient;
	ds::SerialRunnable<FileWriteRunnable> mSaveService;
};


} // namespace waffles
