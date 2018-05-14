## Overview

DS Cinder has the built-in ability to synchronize the graphics on multiple instances of the same app. You can have n-number of instances on the same machine as well as over a local network. Use this ability for:

* Large scale interactives with tons of screens
* Different size displays connected to the same machine (a projector and a high-density touch screen)
* Mirror the output of one display on another

## Netsync Basics

Comprised of two primary pieces, DS Cinder Netsync uses a **Server** and a **Client.** These two pieces are the same compiled executable running in separate folders that communicate over multicast UDP. A majority of built-in sprite types are setup already for synchronizing over the network. If you're building a relatively simple app that only uses images, base sprites, and text, you won't need to do any special network coding. You'll need to add a little bit of overhead if you have any custom requirements or drawing. We'll get to that in a minute. Anyways, onto the instance architecture types:

### Server

The Server is the "brains" of the operation. This is where all your app's logic runs, where any input happens, any data is loaded, and layouts set. On startup, the server builds all of your sprites in a tree, and when a client connects it will send the entire hierarchy of Sprites to all clients. After that, on each frame the changes to each Sprite are sent to all clients. The system for determining which properties have changed uses **markAsDirty();** While you can run your app as a pure server that doesn't render any actual content, it's not generally recommended, see clientserver below.

### Client

A Client is an instance of the app that primarily just renders content. It can gather mouse clicks and send it back to the server, but TUIO callback is not currently supported. There is a small amount of data sent back to the Server to acknowledge receipt of each frame and send any info back, such as video complete or video status. For the most part, the Clients are dummies.

### ClientServer

A ClientServer is a hybrid of the two above: It's the brains of the operation and it renders content. In most setups, you'll have a single clientserver and one or more clients. If you're using videos and need them to be in sync, you'll need to have a clientserver, as pure servers may not handle the video correctly, since they're not rendered. The only drawback to having a clientserver is that it'll draw about a frame before any clients, due to transmit time. In most use cases, that difference isn't noticeable. 

### Standalone

Standalone is a clientserver, without any of the netsync stuff. This is the default architecture setting, and the most common.

## Setup

Check out the [example client and server](https://github.com/Downstream/ds_cinder/tree/develop/example/example_client_and_server).

Basically, you'll need to have a duplicate project folder setup for both the server (actually a clientserver, but we just say server for simplicity) and the client. That includes the data and settings folders. Separate folders are required because each app reads it's settings from the settings folder at startup. So if you ran both apps from the same folder, they'd both be servers or both clients. 

In engine.xml:

* Specify one app instance as a clientserver, and the other as a client in platform:architecture. 
* Make sure the IP and ports match exactly between clientserver and client
* In some circumstances, you'll want to specify the guid for each instance

```xml
        <!-- if false, won't connect udp sender / listener for server or client -->
        <text name="server:connect" value="true" />

        <!-- the multicast ip address and port of the server. This should be identical between client and server-->
        <text name="server:ip" value="239.255.42.58" />
        <int name="server:send_port" value="10370" />
        <int name="server:listen_port" value="10371" />

        <!-- if this is a server (world engine), a client (render engine) or both (world + render). default ="", 
        which is a standalone -->
        <text name="platform:architecture" value="clientserver" />
	
        <text name="platform:guid" value="specific_client" />
```

You'll also want to check your src_rect and dst_rect on the client and server. Specifying the same src_rect for both the client and server will get you a mirrored mode. The world size in engine.xml is the total size the app is interested in. You can specify a unique src_rect for each client to render the relevant slice of the world on that app. 

## Creating a custom sprite

There's a number of steps to take render your own sprite on a client:

1. Install the Sprite on startup
2. Register the client and server functions
3. Register the custom blob type
4. Mark any custom properties as dirty on the server
5. Write custom properties to the blob
6. On the client, read properties back in the same order
7. Apply those properties to the client sprite and render

In the constructor of your main app class (not setupServer(), as this has to be called only once on both the server and client):

```c++
        mEngine.installSprite([](ds::BlobRegistry& r){CustomSprite::installAsServer(r); },
            [](ds::BlobRegistry& r){CustomSprite::installAsClient(r); });
```

The CustomSprite type is registered in the Engine's blob reading system so it knows what custom read functions to use. On each frame, the blob type is sent from the server to the client, which looks up the custom read attribute function by the blob type character. In short, what you need to know is that you have to install your custom sprite type on app startup.

In the header of CustomSprite:

```c++
        static void					installAsServer(ds::BlobRegistry&);
        static void					installAsClient(ds::BlobRegistry&);
```

In the implementation of CustomSprite:

```c++
        namespace {
            char				BLOB_TYPE = 0;
            const ds::ui::DirtyState&	DIRTYSETTING = ds::ui::INTERNAL_A_DIRTY;
            const char					SETTING_ATT = 81;
        }

        void CustomSprite::installAsServer(ds::BlobRegistry& registry) {
            BLOB_TYPE = registry.add([](ds::BlobReader& r) {ds::ui::Sprite::handleBlobFromClient(r); });
        }

        void CustomSprite::installAsClient(ds::BlobRegistry& registry) {
            BLOB_TYPE = registry.add([](ds::BlobReader& r) {ds::ui::Sprite::handleBlobFromServer<CustomSprite>(r); });
        }
        
        CustomSprite::CustomSprite(ds::ui::SpriteEngine& g)
            : ds::ui::Sprite(g)
        {
            mBlobType = BLOB_TYPE;
        }
```

A couple important points to note here: 

* The base sprite handleBlobeFromClient() and handleBlobFromServer() should always be used in those callbacks. The base sprite functions route the messages to the correct sprites
* In the installAsClient() piece, the CustomSprite type is specified in the template for handleBlobFromServer(). That bit is how the client can instantiate your CustomSprite on the client side
* In the constructor, the blob type is applied to the base sprite property of mBlobType, so the sprite knows about this specific type
* Dirty state and attributes are declared in the anonymous namespace. 
* Attributes need to be unique for your sprite, so they just need to not collide with the base sprite attributes, which are roughly from 1 - 40

On the server:

```c++
        void CustomSprite::setCustomSettings(const int numSettings){
            mNumSettings = numSettings;
            markAsDirty(DIRTYSETTING);
        }
```

When your app logic requires changing something custom in your custom sprite, you'll call a function similar to the above. Note that you don't need to do anything special to handle any built-in properties of Sprites, such as color, position, transparency, opacity, scale, etc. Marking a property as dirty is your app's way of knowing that it needs to be sent across the network in the next update.

```c++
        void CustomSprite::writeAttributesTo(ds::DataBuffer& buf) {
	    ds::ui::Sprite::writeAttributesTo(buf);
	
	    if(mDirty.has(DIRTYSETTING)){
		buf.add(SETTING_ATT );
                buf.add(mNumSettings);
                buf.add(mAFakeSampleSecondPropertyString);
             }
        }
```

The base sprite attributes are written to the packet, then any custom properties are written to the packet. The order that you add actual properties to the buf is important. The client needs to read them back out in the same order. 

writeAttributesTo() is called when a new world packet is created (when a client starts) and every frame afterwards. By default, on a new world packet, all bits of the dirty setting are marked, so all properties will be written on new world packets. 

Now that you've added stuff to the packet on the server side and your sprite is installed and registered, data is being sent over to any clients. Here's where you'll handle stuff on the client:

```c++
        void CustomSprite::readAttributeFrom(const char attributeId, ds::DataBuffer& buf) {
            if(attributeId == SETTING_ATT ) {
                mNumSettings = buf.read<int>();
                mAFakeSampleSecondPropertyString = buf.read<std::string>();
                mNeedsBatchUpdate = true;

            } else {
                ds::ui::Sprite::readAttributeFrom(attributeId, buf);
            }
        }
```

Note that the attribute character itself (SETTINGS_ATT in this case) is automatically read from the buffer. The readAttributeFrom() function is called based on the sprite id in the overall packet. Also notice that mNumSettings is both written and read from the packet before mAFakeSampleSecondPropertyString. mNeedsBatchUpdate is a base sprite property that tells the sprite is needs to rebuild the mRenderBatch. For your custom sprite, you'll want to call whatever functions or set any properties for the new settings to take effect. From here, you'll take action on the new mNumSettings and mAFakeSampleSecondPropertyString properties and render appropriately. 

If you need to add children sprites to your custom sprite, do it on the server side. On the client, you'll be able to inspect the children, which I'd recommend be done on the updateServer loop. It's possible you could get attributes from the server before your children are added, so keep that in mind. You could potentially pass across the sprite id of your child sprite and look it up on the client that way. You may need to have children sprites for things like getting the texture output of a video or image, etc.

```c++
        void CustomSprite::onBuildRenderBatch() {
             auto theCircle = ci::geom::Ring().radius(getWidth()).width(1.0f).center(ci::vec2(getWidth(), getWidth())).subdivisions(mNumSettings);

            if(mRenderBatch) mRenderBatch->replaceVboMesh(ci::gl::VboMesh::create(theCircle));
            else mRenderBatch = ci::gl::Batch::create(theCircle, mSpriteShader.getShader());
        }
```

I recommend taking a look at the Example client and server project in the examples folder. It'll let you mess with some of the properties to figure out what's goin on.


## Gotchas and important things to remember

* The IP addresses you pick for the client and server in engine.xml need to be in the multicast range. Note that these ARE NOT physical IP addresses of a machine, but rather a group that is joined where many machines can get the same packet. These addresses are generally of the format 239.xxx.xxx.xxx
* You'll need a unique IP for each app running on the same network. If you have two installs of the same app on the same network with the same IP, they'll both try to connect to different server instances and it's not a happy situation.
* Don't add any sprites on the client side, add them on the server side and inspect the children later in the update loop
* When moving, scaling or rotating sprites, try to use the highest sprite in the hierarchy that makes sense. It'll cut down on net traffic
* Test your app in clientserver -> client mode frequently to catch any issues
* Sprite shaders might need to be built as custom sprites, or loaded from files, and uniforms aren't currently supported across network traffic (you can send them yourself with a custom sprite type though)
* To pass data back to the server from a client use writeClientAttributesTo() and readClientAttributesFrom(), which work the same way as writeAttributesTo() and readAttributesFrom(), just in reverse. Though you will need to add a terminator character at the end of each attribute. See gst_video for an example.
