#include "stdafx.h"

#include "tuio_input.h"

#include <cinder/tuio/Tuio.h>
#include <ds/app/engine/engine.h>

namespace ds {
namespace ui {

TuioInput::TuioInput(ds::ui::SpriteEngine& engine, const int port, const ci::vec2 scaley, const ci::vec2 possy,
					 const float rotty, const int fingerIdOffset, const ci::Rectf& allowedRect)
	: mEngine(engine)
	, mFingerIdOffset(fingerIdOffset)
	, mAllowedRect(allowedRect)
	, mTuioUdpSocket(std::make_shared<ci::osc::ReceiverUdp>(port))
	, mTuioReceiver(std::make_shared<ci::tuio::Receiver>(engine.getWindow(), mTuioUdpSocket.get()))
{
	mTransform = glm::translate(glm::vec3(possy, 0.0f)) *
		glm::rotate(glm::radians(rotty), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::scale(glm::vec3(scaley, 1.0f));

	const auto window = ci::app::getWindow();
	const auto transformCursor = [this, window](const ci::tuio::Cursor2d& tuioCursor) {
		const auto touch = tuioCursor.convertToTouch(window);
		const auto pos = transformEventPosition(touch.getPos(), true);
		const auto prevPos = transformEventPosition(touch.getPrevPos(), true);
		const ci::app::TouchEvent::Touch convertedTouch(pos, prevPos, tuioCursor.getSessionId() + mFingerIdOffset, touch.getTime(), (void *)touch.getNative());
		return ds::ui::TouchEvent(ci::app::getWindow(), { convertedTouch }, true);
	};

	const auto transformObject = [this](const ci::tuio::Object2d& o) {
		const auto pos = transformEventPosition(o.getPosition());
		// Technically, the velocity should be probably transformed too...?
		return ds::TuioObject(o.getClassId(), pos, o.getAngle(), o.getVelocity(), o.getRotationVelocity());
	};

	// Inject transformed touches, ignoring touch-add events that fall outside the allowed rectangle
	mTuioReceiver->setAddedFn   <ci::tuio::Cursor2d>( [=] (const auto& tuioCursor) {
		const ds::ui::TouchEvent& transformed = transformCursor(tuioCursor);
		const auto pos = transformed.getTouches()[0].getPos();
		if (mAllowedRect.calcArea() > 0.0f && !mAllowedRect.contains(pos)) {
			DS_LOG_VERBOSE(1, "TuioInput: discarding touch point at " << pos << " because it doesn't fit in the filter rect " << mAllowedRect);
		}
		else {
			mEngine.injectTouchesBegin(transformed);
		}
	});
	mTuioReceiver->setUpdatedFn<ci::tuio::Cursor2d>( [=] (const auto& tuioCursor) {
		mEngine.injectTouchesMoved(transformCursor(tuioCursor));
	});
	mTuioReceiver->setRemovedFn<ci::tuio::Cursor2d>( [=] (const auto& tuioCursor) {
		mEngine.injectTouchesEnded(transformCursor(tuioCursor));
	});

	// Get those TUIO Objects too
	mTuioReceiver->setAddedFn<ci::tuio::Object2d>(   [this, transformObject](const auto& tuioObject) {
		const auto transformed = transformObject(tuioObject);
		DS_LOG_INFO("TuioInput: TUIO OBJECT ADDED: pos=" << tuioObject.getPosition() 
			<< ", transformed=" << transformed.getPosition()
		);
		mEngine.injectObjectsBegin(transformed);
	});
	mTuioReceiver->setUpdatedFn<ci::tuio::Object2d>( [this, transformObject](const auto& tuioObject) {
		mEngine.injectObjectsMoved(transformObject(tuioObject));
	});
	mTuioReceiver->setRemovedFn<ci::tuio::Object2d>( [this, transformObject](const auto& tuioObject) {
		mEngine.injectObjectsEnded(transformObject(tuioObject));
	});

	// Start listening on the given port
	try {
		DS_LOG_INFO("Starting TuioInput at port "
					<< port << " with offset of " << possy << ", scale of " << scaley << ", and a rotation of " << rotty
					<< " degrees.  finger Id offset of " << mFingerIdOffset << " filter rect of " << mAllowedRect);
		mTuioUdpSocket->bind();
	} catch(std::exception& e) {
		DS_LOG_WARNING("Couldn't connect a tuio input at port " << port << " exception: " << e.what());
	}

	mTuioUdpSocket->listen( 
		[]( asio::error_code ec, asio::ip::udp::endpoint ep ) -> bool {
		if (ec) {
			DS_LOG_WARNING( "TuioInput Error on listener: " << ec.message() << " Error Value: " << ec.value() );
			return false;
		}
		else
			return true;
	} );
}

TuioInput::~TuioInput() {
	mTuioReceiver->clear<ci::tuio::Cursor2d>();
	mTuioReceiver->clear<ci::tuio::Object2d>();
	try {
		mTuioUdpSocket->close();
	} catch(std::exception& e) {
		DS_LOG_WARNING("TuioInput: Couldn't disconnect a tuio input exception: " << e.what());
	}
}

ci::vec2 TuioInput::transformEventPosition(const ci::vec2& pos, const bool doWindowScale) {
	// TUIO Touch events from cinder are scaled by the window size,
	// so we need to reverse this transformation to get world coordinates.
	const auto windowScale = doWindowScale ? mEngine.getDstRect().getSize() : ci::vec2(1.0f);
	return glm::vec2(mTransform * glm::vec4(pos / windowScale, 0.0f, 1.0f));
}

}  // !namespace ui
}  // !namespace ds
