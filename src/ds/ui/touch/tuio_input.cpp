#include "stdafx.h"

#include "tuio_input.h"

#include <ds/app/engine/engine.h>

namespace ds {
namespace ui {

TuioInput::TuioInput(ds::ui::SpriteEngine& engine, const int port, const ci::vec2 scaley, const ci::vec2 possy,
					 const float rotty, const int fingerIdOffset, const ci::Rectf& allowedRect)
	: mEngine(engine)
	, mFingerIdOffset(fingerIdOffset)
	, mAllowedRect(allowedRect) {
	mTransform = glm::translate(glm::vec3(possy, 0.0f)) *
		glm::rotate(glm::radians(rotty), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::scale(glm::vec3(scaley, 1.0f));

	mTuioClient.registerTouchesBegan([this](ci::app::TouchEvent e) {		
		mEngine.injectTouchesBegin(convertTouchEvent(e, true));
	});

	mTuioClient.registerTouchesMoved([this](ci::app::TouchEvent e) { 
		mEngine.injectTouchesMoved(convertTouchEvent(e, false)); 
	});

	mTuioClient.registerTouchesEnded([this](ci::app::TouchEvent e) { 
		mEngine.injectTouchesEnded(convertTouchEvent(e, false)); 
	});

	// Get those TUIO Objects too
	mTuioClient.registerObjectAdded([this](ci::tuio::Object o) {
		const auto transformed = transformEventPosition(o.getPos());
		DS_LOG_INFO("TUIO OBJECT ADDED: pos=" << o.getPos() << ", transformed=" << transformed);

		auto tuioObject = ds::TuioObject(o.getFiducialId(), transformed, o.getAngle());
		mEngine.injectObjectsBegin(tuioObject);
	});

	mTuioClient.registerObjectUpdated([this](ci::tuio::Object o) {
		const auto transformed = transformEventPosition(o.getPos());
		auto       tuioObject =
			ds::TuioObject(o.getFiducialId(), transformed, o.getAngle(), o.getSpeed(), o.getRotationSpeed());
		mEngine.injectObjectsMoved(tuioObject);
	});

	mTuioClient.registerObjectRemoved([this](ci::tuio::Object o) {
		const auto transformed = transformEventPosition(o.getPos());
		auto       tuioObject = ds::TuioObject(o.getFiducialId(), transformed, o.getAngle());
		mEngine.injectObjectsEnded(tuioObject);
	});

	try {
		DS_LOG_INFO("Starting TuioInput at port "
					<< port << " with offset of " << possy << ", scale of " << scaley << ", and a rotation of " << rotty
					<< " degrees.  finger Id offset of " << mFingerIdOffset << " filter rect of " << mAllowedRect);
		mTuioClient.connect(port);
	} catch(std::exception& e) {
		DS_LOG_WARNING("Couldn't connect a tuio input at port " << port << " exception: " << e.what());
	}
}

TuioInput::~TuioInput() {
	if(mTuioClient.isConnected()) {
		try {
			mTuioClient.disconnect();
		} catch(std::exception& e) {
			DS_LOG_WARNING("TuioInput: Couldn't disconnect a tuio input exception: " << e.what());
		}
	}
}

ds::ui::TouchEvent TuioInput::convertTouchEvent(ci::app::TouchEvent& e, const bool isAdd) {
	std::vector<ci::app::TouchEvent::Touch> touches;
	for(const auto& touch : e.getTouches()) {
		const auto windowScale = mEngine.getDstRect().getSize();
		ci::vec2   pos = transformEventPosition(touch.getPos(), true);
		ci::vec2   prevPos = transformEventPosition(touch.getPrevPos(), true);

		// we're only kicking out add points in the rectangle
		if(isAdd && mAllowedRect.getWidth() > 0.0f && mAllowedRect.getHeight() > 0.0f && !mAllowedRect.contains(pos)) {
			DS_LOG_VERBOSE(1, "TuioInput: discarding touch point at " << pos << " because it doesn't fit in the filter rect " << mAllowedRect);
			continue;
		}

		//	std::cout << "TouchInput event: " << touch.getPos() << " " << possy << std::endl;
		touches.push_back(ci::app::TouchEvent::Touch(pos, prevPos, touch.getId() + mFingerIdOffset, touch.getTime(),
			(void*)touch.getNative()));
	}

	return ds::ui::TouchEvent(e.getWindow(), touches, true);
}


ci::vec2 TuioInput::transformEventPosition(const ci::vec2& pos, const bool doWindowScale) {
	// TUIO events from cinder are scaled by the window size,
	// so we need to reverse this transformation to get world coordinates.
	const auto windowScale = doWindowScale ? mEngine.getDstRect().getSize() : ci::vec2(1.0f);
	return glm::vec2(mTransform * glm::vec4(pos / windowScale, 0.0f, 1.0f));
}


}  // !namespace ui
}  // !namespace ds
