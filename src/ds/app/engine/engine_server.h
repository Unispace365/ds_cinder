#pragma once
#ifndef DS_APP_ENGINE_ENGINESERVER_H_
#define DS_APP_ENGINE_ENGINESERVER_H_

#include "ds/app/blob_reader.h"
#include "ds/app/engine/engine.h"
#include "ds/app/engine/engine_client_list.h"
#include "ds/app/engine/engine_io.h"
#include "ds/network/udp_connection.h"

namespace ds {
class ContentWrangler;

/**
 * \class AbstractEngineServer
 * The Server engine contains all app-side behaviour, but no rendering.
 * This class is getting a little messed up because one subclass needs
 * a GlThread and one needs a GlNoThread, so abstract pretty much everything
 * but that.
 */
class AbstractEngineServer : public Engine {
  public:
	AbstractEngineServer(App&, EngineSettings&, EngineData&, const RootList&, int appMode);
	~AbstractEngineServer() override;

	void installSprite(const std::function<void(BlobRegistry&)>& asServer,
					   const std::function<void(BlobRegistry&)>& asClient) override;

	void setup(App&) override;
	void update() override;
	void draw() override;

	void stopServices() override;

	void spriteDeleted(sprite_id_t) override;

	int getBytesReceived() override;
	int getBytesSent() override;

  private:
	static void receiveHeader(DataBuffer&);
	void		receiveCommand(DataBuffer&);
	static void receiveDeleteSprite(DataBuffer&);
	void		receiveClientStatus(DataBuffer&);
	void		receiveClientInput(DataBuffer&);
	void		onClientStartedCommand(DataBuffer&);
	void		onClientRunningCommand(DataBuffer&);

	void handleMouseTouchBegin(const ci::app::MouseEvent&, int id) override;
	void handleMouseTouchMoved(const ci::app::MouseEvent&, int id) override;
	void handleMouseTouchEnded(const ci::app::MouseEvent&, int id) override;

	EngineClientList mClients;

	UdpConnection	 mSendConnection;
	UdpConnection	 mReceiveConnection;
	EngineSender	 mSender;
	EngineReceiver	 mReceiver;
	BlobReader		 mBlobReader;
	ContentWrangler* mContentWrangler;

	/// STATES
	class State {
	  public:
		virtual ~State() = default;

		virtual void begin(AbstractEngineServer&)  = 0;
		virtual void update(AbstractEngineServer&) = 0;
		virtual void spriteDeleted(sprite_id_t) {}

	  protected:
		static void addHeader(DataBuffer&, int frame);
	};

	/* Default state: Gathers all changes in the app and sends them out each frame.
	 */
	class RunningState final : public State {
	  public:
		RunningState();
		void begin(AbstractEngineServer&) override;
		void update(AbstractEngineServer&) override;
		void spriteDeleted(sprite_id_t) override;

		std::vector<sprite_id_t> mDeletedSprites;

	  private:
		void addDeletedSprites(DataBuffer&) const;

		int32_t mFrame;
	};

	/* This state is used to send a client started reply.
	 */
	class ClientStartedReplyState final : public State {
	  public:
		ClientStartedReplyState();
		void				 clear();
		void				 begin(AbstractEngineServer&) override;
		void				 update(AbstractEngineServer&) override;
		std::vector<int32_t> mClients;
	};

	/* This state is used to send the entire world info. It is the initial default
	 * state, and becomes the active state whenever a client requests the world.
	 * It sends out the world once, then moves to the running state.
	 */
	class SendWorldState final : public State {
	  public:
		SendWorldState() = default;
		void begin(AbstractEngineServer&) override;
		void update(AbstractEngineServer&) override;
	};

	State*					mState;
	RunningState			mRunningState;
	ClientStartedReplyState mClientStartedReplyState;
	SendWorldState			mSendWorldState;

	void setState(State&);
};


/**
 * \class EngineServer
 * The Server engine contains all app-side behaviour, but no rendering.
 */
class EngineServer : public AbstractEngineServer {
  public:
	EngineServer(App&, EngineSettings&, EngineData&, const RootList&);
};

} // namespace ds

#endif
