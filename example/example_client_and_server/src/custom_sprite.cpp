#include "custom_sprite.h"

#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"
#include "ds/app/environment.h"
#include "ds/data/data_buffer.h"
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"
#include "ds/ui/sprite/sprite_engine.h"
#include <map>

#include <gl/GL.h>

using namespace ci;

namespace ds { namespace ui {

	namespace {

		/// Blob type is a unique character that identifies this sprite type to the engine
		char BLOB_TYPE = 0;

		/// A custom dirty state to be marked when something custom changes
		/// You can have many of these, depending on how many properties you want to send over
		/// Having specific dirty states per property helps limit net traffic by not sending everything every frame
		const DirtyState& NUM_SEGMENTS_DIRTY  = INTERNAL_A_DIRTY;
		const DirtyState& NUM_INSTANCES_DIRTY = INTERNAL_B_DIRTY;

		/// ATT is an Attribute to be sent. This char is sent over the net to identify what property follows
		const char NUM_SEGS_ATT = 83;
		const char NUM_INST_ATT = 84;

	} // namespace

	// Part of the registration mechanism
	void CustomSprite::installAsServer(ds::BlobRegistry& registry) {
		BLOB_TYPE = registry.add([](BlobReader& r) { Sprite::handleBlobFromClient(r); });
	}

	void CustomSprite::installAsClient(ds::BlobRegistry& registry) {
		BLOB_TYPE = registry.add([](BlobReader& r) { Sprite::handleBlobFromServer<CustomSprite>(r); });
	}

	CustomSprite::CustomSprite(SpriteEngine& engine)
	  : ds::ui::Sprite(engine)
	  , mNumberOfSegments(0)
	  , mNumberOfInstances(1) {
		// The base sprite mBlobType tracks this specific sprite type in the netsync system
		// This is pretty important
		mBlobType = BLOB_TYPE;
		setTransparent(false);
		mLayoutFixedAspect = true;
	}

	// This is called on both the client and the server
	// Shove our custom drawing stuff into the base render batch to be drawn at drawLocalClient time
	void CustomSprite::onBuildRenderBatch() {
		if (getWidth() <= 0.0f) return;


		auto theCircle = ci::geom::Ring()
							 .radius(getWidth())
							 .width(1.0f)
							 .center(ci::vec2(getWidth(), getWidth()))
							 .subdivisions(mNumberOfSegments);

		if (mRenderBatch)
			mRenderBatch->replaceVboMesh(ci::gl::VboMesh::create(theCircle));
		else
			mRenderBatch = ci::gl::Batch::create(theCircle, mSpriteShader.getShader());
	}

	// Custom drawing - aka the whole reason to custom netsync stuff
	void CustomSprite::drawLocalClient() {
		if (mRenderBatch) {
			float offsety = 3.0f;
			for (int i = 0; i < mNumberOfInstances; i++) {
				mRenderBatch->draw();
				ci::gl::translate(offsety, offsety);
			}
		}
	}

	// This only gets called if you're running a server only instance (untested)
	void CustomSprite::drawLocalServer() {
		drawLocalClient();
	}

	// Called every frame on the server
	// Collects stuff to be sent to clients
	void CustomSprite::writeAttributesTo(ds::DataBuffer& buf) {
		ds::ui::Sprite::writeAttributesTo(buf);

		// If the number of segments or instances changes since the last frame, this will be true
		// So we bundle up our changes and send them over
		// The string is just for this example, for how to send a string
		if (mDirty.has(NUM_SEGMENTS_DIRTY)) {
			std::string ohHeyThere = "oh, hey there.";
			buf.add(NUM_SEGS_ATT);
			buf.add(mNumberOfSegments);
			buf.add(ohHeyThere);
		}

		if (mDirty.has(NUM_INSTANCES_DIRTY)) {
			buf.add(NUM_INST_ATT);
			buf.add(mNumberOfInstances);
		}
	}

	// This is only called on the client
	// This is called once for each attribute
	// Important: you need to read all the attributes out that you put in above
	// Otherwise the packet won't be read correctly and bad things can happen
	// Note that the super class readAttributeFrom() is called if we don't find a custom one
	// That's for base sprite stuff like size, position, color, etc
	void CustomSprite::readAttributeFrom(const char attributeId, ds::DataBuffer& buf) {
		if (attributeId == NUM_SEGS_ATT) {
			mNumberOfSegments		 = buf.read<int>();
			std::string sampleString = buf.read<std::string>();
			std::cout << "Segments updated: " << sampleString << std::endl;
			mNeedsBatchUpdate = true;

		} else if (attributeId == NUM_INST_ATT) {
			mNumberOfInstances = buf.read<int>();
			mNeedsBatchUpdate  = true;

		} else {
			ds::ui::Sprite::readAttributeFrom(attributeId, buf);
		}
	}

	// Called on the server
	// Note the markAsDirty to alert our writeAttributesTo() function to pick up these changes
	void CustomSprite::setNumberOfSegments(const int numSegments) {
		if (mNumberOfSegments == numSegments) return;
		mNumberOfSegments = numSegments;
		mNeedsBatchUpdate = true;
		markAsDirty(NUM_SEGMENTS_DIRTY);
	}

	void CustomSprite::setNumberOfInstances(const int numInstances) {
		if (numInstances == mNumberOfInstances) return;
		mNumberOfInstances = numInstances;
		markAsDirty(NUM_INSTANCES_DIRTY);
	}
}} // namespace ds::ui
