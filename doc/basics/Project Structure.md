# Project Structure

## Projects

Projects are how different components of ds_cinder are separated and managed. Significant pieces like Video and PDF are in their own projects so they can be updated or replaced without significant changes to the base platform.

Individual projects are included in your project through Property Sheets which link to the appropriate headers and libraries. So you could remove the project references from the Solution as long as you include the Property Sheets and have each project compiled.


### Your App project

The project file specific to your app. This is where your custom code lives, along with font files, settings, layout files, and app resources like icons and images. 


### platform

Platform is the base framework for ds_cinder. It's the only required project, and includes the basics:

* App: the basic app runner and bootstrapper for ds_cinder 
* Engine: manages the sprite hierarchy, runs updates, and deals with netsync, if enabled
* Sprite: root display object, touch handling, drawing, animation, and hierarchy
* Basic sprite types: such as circle, border, layout, buttons and more
* Image: loading images and display
* Query: data model and querying support
* Etc: Lots of other utils, conveniences and basic functions


### essentials

A collection of helpers and conveniences for faster app development.

* Curl / httpsClient: for easy connectivity to foreign api's
* InterfaceXmlImporter: loads xml layouts into the app
* SmartLayout: automatically loads xml layouts into a Layout sprite
* UI elements: like donut arc, png sequences, line, dashed line, 5-finger touch menu, drawing area, scroll area, scroll list
* Automator: to automate app interactivity for debugging


### physics

Uses Box2d to add simple 2d physics to your app. Creates the physics world and binds physics bodies to sprites.


### pdf

Load PDFs as a sprite via muPDF. Handles multiple pages, multiple page sizes and dynamic re-rendering. Doesn't support many advanced PDF functions like forms, links, video, flash, etc.


### video

Uses GStreamer to load and display videos. Supports up to 4k 30fps videos, video streams, multi-computer sync and 360-degree panoramic videos.


### cef_web

Uses CEF (chromium embedded framework) to load and display websites. Supports touch, mouse and keyboard input, as well as WebGL and most HTML5 functions. 


### viewers

Some helpful ui classes to load media (Web, PDF, Image, Video) with simple interfaces for play/pause, page control, volume, forward/back, etc. Also has simple viewer classes for a scalable, moveable, bounds-checked window. If you want to easily display media that can be interacted with, this project is recommended.


### mosquitto

Uses mosquitto to connect to MQTT servers. Subscribe and publish mqtt messages
