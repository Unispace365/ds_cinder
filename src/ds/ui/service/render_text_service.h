#pragma once
#ifndef DS_UI_SERVICE_RENDERTEXTSERVICE_H_
#define DS_UI_SERVICE_RENDERTEXTSERVICE_H_

#include <memory>
#include <vector>
#include <unordered_map>
#include <cinder/Thread.h>
#include <cinder/gl/Texture.h>
#include "ds/thread/gl_thread.h"

namespace ds {

namespace ui {
class RenderTextService;

/**
 * \class ds::ui::RenderTextShared
 * Information shared between the text object and the worker thread/object.
 */
class RenderTextShared {
public:
	RenderTextShared();

	void			setLatestRequestId(const int);
	int				getLatestRequestId();

private:
	std::mutex		mLock;
	// This stores the most recent ID that a client has requested
	// be created. It is used in case a lot of requests are made,
	// so we can toss all but the latest.
	int				mLatestRequestId;
};

/**
 * \class ds::ui::RenderTextFinished
 * Provide the results of the operation.
 */
class RenderTextFinished {
public:
	RenderTextFinished();

	ci::gl::Texture	mTexture;
};

/**
 * \class ds::ui::RenderTextClient
 */
class RenderTextClient {
public:
	RenderTextClient(RenderTextService&, const std::function<void(RenderTextFinished&)>& finishedFn);
	~RenderTextClient();

	void				start(std::weak_ptr<RenderTextShared> shared);

private:
	RenderTextService&	mService;
};

/**
 * \class ds::ui::RenderTextWorker
 * \brief Render a block of text.
 */
class RenderTextWorker {
public:
	RenderTextWorker();
	RenderTextWorker(	const void* clientId,
						std::weak_ptr<RenderTextShared>);

	const void*			mClientId;
	// A reference to the current
	std::weak_ptr<RenderTextShared>
						mShared;
	RenderTextFinished	mFinished;
};

/**
 * \class ds::ui::RenderTextService
 */
class RenderTextService : public ds::GlThreadClient<RenderTextService> {
public:
	RenderTextService(GlThread&);

	void					registerClient(	const void*,
											const std::function<void(RenderTextFinished&)>&);
	void					unregisterClient(const void*);

	void					start(	const void* clientId,
									std::weak_ptr<RenderTextShared> shared);

	void					update();

private:
	std::unordered_map<const void*, std::function<void(RenderTextFinished&)>>
							mClientRegistry;

	std::mutex				mLock;
	std::vector<RenderTextWorker>
							mInput, mOutput,
							mMainThreadTmp,
							mWorkerThreadTmp;

	void					_run();
};

} // namespace ui

} // namespace ds

#endif // DS_UI_SERVICE_RENDERTEXTSERVICE_H_