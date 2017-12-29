#pragma once
#ifndef DS_APP_ENGINE_ENGINESERVER_H_
#define DS_APP_ENGINE_ENGINESERVER_H_

#include "ds/app/blob_reader.h"
#include "ds/app/engine/engine.h"
#include "ds/app/engine/engine_client_list.h"
#include "ds/app/engine/engine_io.h"
#include "ds/network/udp_connection.h"
#include "ds/thread/work_manager.h"
#include "ds/ui/service/load_image_service.h"

namespace ds {

/**
 * \class ds::AbstractEngineServer
 * The Server engine contains all app-side behaviour, but no rendering.
 * This class is getting a little messed up because one subclass needs
 * a GlThread and one needs a GlNoThread, so abstract pretty much everything
 * but that.
 */
class AbstractEngineServer : public Engine {
public:
	AbstractEngineServer(ds::App&, const ds::EngineSettings&, ds::EngineData&, const ds::RootList&);
	~AbstractEngineServer();

	virtual ds::WorkManager&		getWorkManager()		{ return mWorkManager; }

	virtual void					installSprite(	const std::function<void(ds::BlobRegistry&)>& asServer,
													const std::function<void(ds::BlobRegistry&)>& asClient);

	virtual void					setup(ds::App&);
	virtual void					update();
	virtual void					draw();

	virtual void					stopServices();
	virtual int						getMode() const { return SERVER_MODE; }

	virtual void					spriteDeleted(const ds::sprite_id_t&);

	virtual int						getBytesRecieved();
	virtual int						getBytesSent();

private:
	void							receiveHeader(ds::DataBuffer&);
	void							receiveCommand(ds::DataBuffer&);
	void							receiveDeleteSprite(ds::DataBuffer&);
	void							receiveClientStatus(ds::DataBuffer&);
	void							receiveClientInput(ds::DataBuffer&);
	void							onClientStartedCommand(ds::DataBuffer&);
	void							onClientRunningCommand(ds::DataBuffer&);

	virtual void					handleMouseTouchBegin(const ci::app::MouseEvent&, int id);
	virtual void					handleMouseTouchMoved(const ci::app::MouseEvent&, int id);
	virtual void					handleMouseTouchEnded(const ci::app::MouseEvent&, int id);

	typedef Engine inherited;
	WorkManager						mWorkManager;
	EngineClientList				mClients;

	ds::UdpConnection				mSendConnection;
	ds::UdpConnection				mReceiveConnection;
	EngineSender					mSender;
	EngineReceiver					mReceiver;
	ds::BlobReader					mBlobReader;

	// STATES
	class State {
	public:
		State();
		virtual void				begin(AbstractEngineServer&);
		virtual void				update(AbstractEngineServer&) = 0;
		virtual void				spriteDeleted(const ds::sprite_id_t&) { }

	protected:
		void						addHeader(ds::DataBuffer&, const int frame);
	};

	/* Default state: Gathers all changes in the app and sends them out each frame.
	 */
	class RunningState : public State {
	public:
		RunningState();
		virtual void				begin(AbstractEngineServer&);
		virtual void				update(AbstractEngineServer&);
		virtual void				spriteDeleted(const ds::sprite_id_t&);

		std::vector<sprite_id_t>	mDeletedSprites;
	private:
		void						addDeletedSprites(ds::DataBuffer&) const;

		int32_t						mFrame;
	};

	/* This state is used to send a client started reply.
	 */
	class ClientStartedReplyState : public State {
	public:
		ClientStartedReplyState();
		void						clear();
		virtual void				begin(AbstractEngineServer&);
		virtual void				update(AbstractEngineServer&);
		std::vector<int32_t>		mClients;
	};

	/* This state is used to send the entire world info. It is the initial default
	 * state, and becomes the active state whenever a client requests the world.
	 * It sends out the world once, then moves to the running state.
	 */
	class SendWorldState : public State {
	public:
		SendWorldState();
		virtual void				begin(AbstractEngineServer&);
		virtual void				update(AbstractEngineServer&);
	};

	State*							mState;
	RunningState					mRunningState;
	ClientStartedReplyState			mClientStartedReplyState;
	SendWorldState					mSendWorldState;

	void							setState(State&);
};


/**
 * \class ds::EngineServer
 * The Server engine contains all app-side behaviour, but no rendering.
 */
class EngineServer : public AbstractEngineServer {
public:
	EngineServer(ds::App&, const ds::EngineSettings&, ds::EngineData&, const ds::RootList&);
	~EngineServer();

	virtual ui::LoadImageService&	getLoadImageService()	{ return mLoadImageService; }

private:
	typedef AbstractEngineServer inherited;
	ui::LoadImageService			mLoadImageService;
};

} // namespace ds

#endif
