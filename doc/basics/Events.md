# Events


## Basics

Events are things that happen in the app! Declare an event in c++:

    class SlideForwardRequest : public ds::RegisteredEvent<SlideForwardRequest> {};

That simply lets the system know that that could be an event, and it is assigned a unique identifier. To listen to events in a part of your app, you'll need an event client on your class, and ask it to listen for specific events:

    mEventClient.listenToEvents<SlideForwardRequest>([this](auto e) { updateTheViewOrSomething(); });

Ok, sweet! Now we're set up to get notified of incoming events. So how do the events go out? To notify the system of that event, call it like this:

    mEngine.getNotifier().notify( SlideForwardRequest() );

By default, all events are global and are sent to all event clients, so use them carefully if your app contains multiple duplicate interfaces for multiple users. For instance, if you had 2 slide viewers in the above case, they would both have been notified of that event and updated their view (or something). One way to handle that is to have some interfaces use lambdas to communicate locally (perhaps from a tap callback?) or use parameters on the Event class. There are a few parameters you can use already on Events:

````c++
	SlideForwardRequest theRequest;
	theRequest.mUserId = mPlacematIdOrContentIdOrSomething;
	theRequest.mEventOrigin = getGlobalLocation();
	theRequest.mSpriteOriginator = this;
	mEngine.getNotifier().notify(theRequest);
	
	// later, in another class...
	mEventClient.listenToEvents<SlideForwardRequest>([this](auto e) { 
		if(mPlacematIdOrContentIdOrSomething == e.mUserId){
			updateTheViewOrSomething();
		}
	});
````

If you use a layout class to dispatch an event using **on_tap_event** or **on_click_event**, the sprite originator and origin will be automatically applied.

Registering an event to be called by name in a layout:

    ds::event::Registry::get().addEventCreator(SlideForwardRequest::NAME(), [this]()->ds::Event*{return new SlideForwardRequest(); });

	
## Built-in Events

There's some handy events already built into ds_cinder. The ones with "Request" in the name can generally be notified from you client app to make ds_cinder do stuff.

* **ds::app::EngineStateEvent**: In a client/server setup, notifies when the engine state has changed
* **ds::app::IdleEndedEvent**: The app got some new interaction and has exited idle mode
* **ds::app::IdleStartedEvent**: The app hasn't had interacting since idle_seconds elapse and has started idling
* **ds::app::RequestAppExitEvent**: Send from anywhere in your client app to exit the app immediately

* **ds::EngineStatsView::ToggleStatsRequest**: Show/hide the status pane in the upper left corner of the app

* **ds::cfg::Settings::SettingsEditedEvent**: A setting has been altered in the settings editor (e). This event comes with the settings file and the name of the setting that was changed. Useful for updating your client app on-the-fly with settings changes so you can quickly develop complex shit.
* **ds::ContentUpdatedEvent**: ContentWrangler has updated mEngine.mContent with the latest data. Only called if ContentWrangler has been enabled in engine.xml
* **ds::RequestContentQueryEvent**: A request of ContentWrangler to start a new query. Queries are returned asynchronously. 
* **ds::DsNodeMessageReceivedEvent**: A new message from dsnode has been received. Only called if ContentWrangler's node watching has been enabled and you're running DsNode (proprietary)
* **ds::DirectoryWatcher::Changed**: A windows directory that's being watched has been modified in some way. Automatically setup if you have auto_refresh_app or auto_refresh_directories set in engine.xml
