#include "ds/ui/service/render_text_service.h"

#include <cinder/ImageIo.h>
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"
#include "ds/ui/sprite/image.h"
#include "ds/ui/sprite/fbo/fbo.h"

// tmp
#include <cinder/Surface.h>
#include <cinder/ImageIo.h>


namespace {
const ds::BitMask     RENDER_TEXT_LOG_M = ds::Logger::newModule("render_text");
}

namespace ds {
namespace ui {

/**
 * \class ds::ui::RenderTextShared
 */
RenderTextShared::RenderTextShared()
	: mLatestRequestId(-1)
{
}

void RenderTextShared::setLatestRequestId(const int id)
{
    boost::lock_guard<boost::mutex> lock(mLock);
	mLatestRequestId = id;
}

int RenderTextShared::getLatestRequestId()
{
    boost::lock_guard<boost::mutex> lock(mLock);
	return mLatestRequestId;
}

/**
 * \class ds::ui::RenderTextFinished
 */
RenderTextFinished::RenderTextFinished()
{
}

/**
 * \class ds::ui::RenderTextClient
 */
RenderTextClient::RenderTextClient(	RenderTextService& s,
									const std::function<void(RenderTextFinished&)>& finishedFn)
	: mService(s)
{
	mService.registerClient(this, finishedFn);
}

RenderTextClient::~RenderTextClient()
{
	mService.unregisterClient(this);
}

void RenderTextClient::start(std::weak_ptr<RenderTextShared> shared, int code)
{
	mService.start(this, shared, code);
}

/**
 * \class ds::ui::RenderTextWorker
 */
RenderTextWorker::RenderTextWorker()
	: mClientId(nullptr)
{
}

RenderTextWorker::RenderTextWorker(	const void* clientId,
									std::weak_ptr<RenderTextShared> shared,
									int code)
	: mClientId(clientId)
	, mShared(shared)
	, mCode(code)
{
}

void RenderTextWorker::clear()
{
	mShared.reset();
	mFinished.mTexture = ci::gl::Texture();
}

/**
 * \class ds::ui::RenderTextService
 */
RenderTextService::RenderTextService(GlThread& t)
	: GlThreadClient<RenderTextService>(t)
{
	mInput.reserve(32);
	mOutput.reserve(32);
	mMainThreadTmp.reserve(32);
	mWorkerThreadTmp.reserve(32);
}

void RenderTextService::registerClient(	const void* clientId,
										const std::function<void(RenderTextFinished&)>& fn)
{
	mClientRegistry[clientId] = fn;
}

void RenderTextService::unregisterClient(const void* clientId)
{
	if (!mClientRegistry.empty()) {
		auto found = mClientRegistry.find(clientId);
		if (found != mClientRegistry.end()) mClientRegistry.erase(found);
	}
}

void RenderTextService::start(	const void* clientId,
								std::weak_ptr<RenderTextShared> shared,
								int code)
{
	{
		boost::lock_guard<boost::mutex> lock(mLock);
		mInput.push_back(std::unique_ptr<RenderTextWorker>(new RenderTextWorker(clientId, shared, code)));
	}
	performOnWorkerThread(&RenderTextService::_run);
}

void RenderTextService::update()
{
	mMainThreadTmp.clear();
	{
		boost::lock_guard<boost::mutex> lock(mLock);
		mMainThreadTmp.swap(mOutput); 
	}
	if (mClientRegistry.empty()) return;
	for (auto it=mMainThreadTmp.begin(), end=mMainThreadTmp.end(); it!=end; ++it) {
		RenderTextWorker*		worker = it->get();
		if (!worker) continue;

		auto found = mClientRegistry.find(worker->mClientId);
		if (found != mClientRegistry.end()) {
			if (found->second) found->second(worker->mFinished);
		}
		worker->clear();
		it->reset();
	}
//      if (glGetError() == GL_OUT_OF_MEMORY) {
//        DS_LOG_ERROR_M("LoadImageService::update() called on filename: " << out.mFilename << " recieved an out of memory error. Image my be too big.", LOAD_IMAGE_LOG_M);
//      }
//      DS_REPORT_GL_ERRORS();
}

void RenderTextService::_run()
{
	// Pop off the items I need
	mWorkerThreadTmp.clear();
	{
		boost::lock_guard<boost::mutex> lock(mLock);
		mInput.swap(mWorkerThreadTmp);
	}

	DS_REPORT_GL_ERRORS();

	FboGeneral		fbo;
	fbo.setup(true);

	for (auto it=mWorkerThreadTmp.begin(), end=mWorkerThreadTmp.end(); it!=end; ++it) {
		RenderTextWorker*	worker = it->get();
		if (!worker) continue;

		ci::gl::Texture::Format format;
		format.setTarget(GL_TEXTURE_2D);
		ci::gl::Texture		tex;
		int					size = 30;
		if (worker->mCode == 900) size = 50;
		tex = ci::gl::Texture(size, size, format);
		worker->mFinished.mTexture = tex;
		worker->mFinished.mCode = worker->mCode;
		{
			FboGeneral::AutoAttach	attach(fbo, tex);
			{
				FboGeneral::AutoRun	run(fbo);
				ci::gl::clear(ci::ColorA(1.0f, 0.0f, 1.0f, 1.0f));

			glPushAttrib(GL_COLOR);
			ci::gl::color(ci::ColorA(0.0f, 1.0f, 0.0f, 1.0f));
			ci::gl::drawSolidRect(ci::Rectf(0.0f, 0.0f, size, size));
			glPopAttrib();

			}
		}

		{
			if (worker->mCode > 0) {
				ci::Surface8u	s(worker->mFinished.mTexture);
				std::stringstream	buf;
				buf << "C:\\Users\\erich\\Documents\\downstream\\wtf_" << worker->mCode << ".png";
				ci::writeImage(buf.str(), s);
			}
		}

		boost::lock_guard<boost::mutex> lock(mLock);
		mOutput.push_back(std::move(*it));
	}
	mWorkerThreadTmp.clear();

#if 0
	// Load them all
	DS_REPORT_GL_ERRORS();
	for (int k=0; k<mTmp.size(); k++) {
		op&						           top = mTmp[k];
    try {
//      DS_LOG_INFO_M("LoadImageService::_load() on file (" << top.mFilename << ")", LOAD_IMAGE_LOG_M);
      top.mSurface = ci::Surface8u(ci::loadImage(top.mFilename));
      DS_REPORT_GL_ERRORS();
      if (top.mSurface) {
        // This is to immediately place operations on the out put...
        Poco::Mutex::ScopedLock		l(mMutex);
        mOutput.push_back(op(top));
      }
    } catch (std::exception const& ex) {
      DS_LOG_WARNING_M("LoadImageService::_load() failed ex=" << ex.what() << " (file=" << top.mFilename << ")", LOAD_IMAGE_LOG_M);
    }
    top.clear();
		DS_REPORT_GL_ERRORS();
	}
	mTmp.clear();
#endif
}

} // namespace ui
} // namespace ds