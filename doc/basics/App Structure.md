# App Structure

## Folders

**data**: where fonts, images, layouts, and data models live
**install**: scripts for building installers
**settings**: xml files that configure your app
**src**: c++ source
**vs2015**: project, solution, and build output


## Structure

By convention, we use an MVC (model view controller) setup for apps. In the past, it's been the responsibility of each app author to write the query and model portion of the app. Now there are classes in ds_cinder to handle that automatically. The model, ContentModelRef, holds the data, such as text data, pointers to media assets, and other information. The query and handler, ContentQuery and ContentWrangler, grab the data out of a sqlite database and send an event to the rest of the app notifying of new data. ContentWrangler can listen to DsNode messages (if you're using DsNode, a Downstream-specific utility) for when to grab new data. 

Your app reads the data out of ContentModelRefs using mEngine.mContent, the root content. From there, you'll write controller classes that decide what to do with the data and load appropriate views. 

## Basic Bootstrapping Flow

1. App startup
2. ContentWrangler reads from content_model.xml for data structure and starts a ContentQuery to get data from the resource_location sqlite database
3. Your controller class(es) are instantiated and added to the root sprites. All controllers extend Sprite so they can be memory managed by the Engine.
4. At some point later, the ContentQuery finishes gathering all required data, ContentWrangler moves that data to mEngine.mContent, and sends a ds::ContentUpdatedEvent event out
5. Your controllers listen to ds::ContentUpdatedEvent to refresh data onscreen
6. Now that your app is up and running, users may interact with the content and navigate around. It's recommended you keep track of the state of the app so you can easily reset the state. 
7. Each time a new view is opened, it should look for the new content in mEngine.mContent, so you can be sure that what Users are looking at is always the most up-to-date content