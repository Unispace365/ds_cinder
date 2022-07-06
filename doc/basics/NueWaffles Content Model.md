# NueWaffles Content Model

## Introduction
The NueWaffles Content Model is a combination of a specific Content Model XML file and two c++ classes to parse the flat SQL data into a tree structure that matches the content tree found a NueWaffles CMS. 

## Set Up
The first step to setup a project is to ensure a copy of nw_content_model.xml is in the projects data/model folder and the engine.xml is pointing to it. You should be able to find a copy of nw_content_model.xml in the FullStarter project.

### Engine.xml
```XML
...
<setting name="content:model_location" value="%APP%/data/model/nw_content_model.xml" type="string" />
...
```
Next is to create an instance of `ds::model::NWQueryHandler` and `ds::model::NWScheduleHandler` in your project.

### my_project_app.h
```c++
#pragma once

#include <cinder/app/App.h>
#include <ds/app/app.h>
#include <ds/app/event_client.h>
#include <ds/content/nw_query_handler.h>
#include <ds/content/nw_schedule_handler.h>

namespace my_project {

class my_project_app : public ds::App {
public:
	my_project_app();

	void				setupServer();

	virtual void		fileDrop(ci::app::FileDropEvent event) override;

private:

	// App events can be handled here
	ds::EventClient		mEventClient;

    //Member variables for QueryHandler and ScheduleHandler
	ds::model::NWQueryHandler		mQueryHandler;
	ds::model::NWScheduleHandler		mScheduleHandler;
};

} // !namespace my_project
```
And initialize them in the constructor.
### my_project_app.cpp
```c++


my_project_app::my_project_app()
	: ds::App()
	, mQueryHandler(mEngine)
	, mScheduleHandler(mEngine)
	
{

	//There is more in the constructor but its not relevent to this set up. The important part here is the mQuereyHandler and mScheduleHandler get initialized
}
```

This is setup the handlers and they will start responding to events as described below.

## Usage

The Query handler will process the content of the database upon receiving the `ds::ContentUpdated` event. upon compleation of the processing it will emit the `ds::CMSDataLoadCompleteEvent` event. Applications should respond to this event instead of `ds::ContentUpdated` event. 

The exception to this is if you need to get the list of CMS events that are happening at the given moment. The Schedule Handler will process the events from the cms every 5 seconds and send out a `ds::ScheduleUpdatedEvent` event. Note that a `ds::ScheduleUpdatedEvent` does not mean that the schedule has changed, just that the processing is complete.

### *Query Handler*
The Query handler will add the following Content Models to the root content model:

- **cms_tags** - the list of tags in the cms
- **cms_root** - this is the content tree. It is unusual to get to part of this model directly, usually you would look up a node that is aquired from an event or a platform node selector in the cms. the Query handler will also as a references map that is called `valid_nodes`
- **cms_platforms** - this is the list of platforms that match the applications platform key. The platform key is set via the engine file's `platform:key` setting. If this value is blank or AUTO then the computers machine name is used for the platform key. Unless you have a reason to do otherwise, you should ensure that only one cms platform will match a give computer as it will be hard to distinguish the correct platform for an application.
- **cms_events** this is a list of **ALL** the cms events for all the schedules for the first platform in the platform list. This includes past cms events and future events. The Schedule handler is responsible to filtering these events to events that are occuring "now"

### *Scheduler Handler*
The Schedule hander will process all events from the `cms_events` model in the root model. It will place a list of all currently happening events in the `current_events` content model as a child of the root content model.

## How to's
---
### **Get a node from a node selector in a platform.**

This is getting the content model of a node from a platform that has node_selector with an app_key of `active_playlist_id`
```c++
//get the platform. It's always the first child.
auto platform = mEngine.mContent.getChildByName("cms_platforms").getChild(0);

//if we have a valid platform....
if (platform) {
	auto validNodes = mEngine.mContent.getChildByName("cms_root").getReferences("valid_nodes");
	
	//get the playlist id.
	auto playlistId = platform.getPropertyInt("active_playlist_id");
	//search the valid nodes and get a interator to the valid_nodes map
	auto playlistItr = validNodes.find(playlistId);

	//we are using a ternary operator to give the content model of the node if found or an empty one if not.
	auto playlist = playlistItr != validNodes.end() ? playlistItr->second : ds::model::ContentModelRef();
}
```

### **Sort cms events based on application logic.**
The schedule that this applications platform is a port of has 3 types of events. Two of which are relevant to this platform. The business logic dicates that we always show a welcome event if there is one and that we give priority to the most recent welcome item. If there are ambient events and there are *NO* welcome events then every ambient event is put into a playlist. In this case that playlist will be represented by a vector. 

```c++
//this is set up by the Schedule Handler
auto currentEvents = mEngine.mContent.getChildByName("current_events").getChildren();

std::vector<ds::model::ContentModelRef> welcomeEvents;
std::vector<ds::model::ContentModelRef> ambientEvents;
std::vector<ds::model::ContentModelRef> showModel;
//do we have any events right now at all?
if(currentEvents.size() > 0) { 
	for (auto& event : currentEvents) {
		
		if (event.getPropertyString("kind") == "welcome_event") {
			welcomeEvents.clear();
			welcomeEvents.push_back(event);
		}
		else if (event.getPropertyString("kind") == "ambient_event") {
			ambientEvents.clear();
			ambientEvents.push_back(event);
		} else {
			//DS_LOG_WARNING("Ignoring event kind: " << event.getPropertyString("kind"));
		}
	}

	if(welcomeEvents.size() > 0) {
		//sort the welcome events and take the first
		std::sort(
			welcomeEvents.begin(), 
			welcomeEvents.end(), 
			[](
				ds::model::ContentModelRef& a, 
				ds::model::ContentModelRef& b) {
				Poco::DateTime a_start, b_start;
				int tzd;
				Poco::DateTimeParser::tryParse(
					a.getPropertyString("effective_time_start"), a_start, tzd);
				Poco::DateTimeParser::tryParse(
					b.getPropertyString("effective_time_start"), b_start, tzd);
				return b_start < a_start; //latest first!
			});
		showModel.pushBack(welcomeEvents.at(0));
	} else if (ambientEvents.size() > 0) {
		//well take all the ambient events..
		showModel = ambientEvents;
	} else {
		//the app will need to decide what to do when there are no events at all.
	}
} 

```

