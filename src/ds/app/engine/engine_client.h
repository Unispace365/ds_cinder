#pragma once
#ifndef DS_APP_ENGINE_ENGINECLIENT_H_
#define DS_APP_ENGINE_ENGINECLIENT_H_

#include "ds/app/blob_reader.h"
#include "ds/app/engine/engine.h"
#include "ds/app/engine/engine_io.h"
#include "ds/app/engine/engine_io_defs.h"
#include "ds/network/udp_connection.h"
#include "ds/ui/service/load_image_service.h"

namespace ds {

/**
 * \class EngineClient
 * The Server engine contains all app-side behaviour, but no rendering.
 */
class EngineClient : public Engine {
  public:
	static char getClientStatusBlob();
	EngineClient(App&, EngineSettings&, EngineData&, const RootList&);
	~EngineClient() override;

	EngineClient(const EngineClient&)			 = delete;
	EngineClient(EngineClient&&)				 = delete;
	EngineClient& operator=(const EngineClient&) = delete;
	EngineClient& operator=(EngineClient&&)		 = delete;

	sprite_id_t nextSpriteId() override;

	void installSprite(const std::function<void(BlobRegistry&)>& asServer,
					   const std::function<void(BlobRegistry&)>& asClient) override;

	void update() override;
	void draw() override;

	void stopServices() override;

	int getBytesReceived() override;
	int getBytesSent() override;

	/// The most recent frame received from the server.
	int32_t mServerFrame;

  private:
	void        receiveHeader(DataBuffer&);
	void        receiveCommand(DataBuffer&);
	void        receiveDeleteSprite(DataBuffer&);
	static void receiveClientStatus(DataBuffer&);
	static void receiveClientInput(DataBuffer&);
	void        onClientStartedReplyCommand(DataBuffer&);

	void handleMouseTouchBegin(const ci::app::MouseEvent&, int id) override;
	void handleMouseTouchMoved(const ci::app::MouseEvent&, int id) override;
	void handleMouseTouchEnded(const ci::app::MouseEvent&, int id) override;
	void sendMouseTouch(int phase, const ci::ivec2& pos);

	EngineIoInfo   mIoInfo;
	UdpConnection  mSendConnection;
	UdpConnection  mReceiveConnection;
	EngineSender   mSender;
	EngineReceiver mReceiver;
	BlobReader	   mBlobReader;
	int32_t		   mSessionId;
	/// True if I lost the connection, renewed it, and am
	/// waiting to hear back.
	bool mConnectionRenewed;

	/// STATES
	class State {
	  public:
		virtual ~State() = default;

		virtual bool getHeaderAndCommandOnly() const = 0;
		virtual void begin(EngineClient&)			 = 0;
		virtual void update(EngineClient&)			 = 0;
	};

	class RunningState final : public State {
	  public:
		RunningState() = default;
		bool getHeaderAndCommandOnly() const override { return false; }
		void begin(EngineClient&) override;
		void update(EngineClient&) override;
	};

	/// I have just started, and am sending the server the
	/// CMD_CLIENT_STARTED command. I will wait here until
	/// I receive CMD_CLIENT_STARTED_REPLY.
	class ClientStartedState final : public State {
	  public:
		ClientStartedState() = default;
		bool getHeaderAndCommandOnly() const override { return true; }
		void begin(EngineClient&) override;
		void update(EngineClient&) override;

	  private:
		/// Avoid flooding the server with requests for the world.
		int mSendFrame{0};
	};

	/// I have no data, and am waiting for a complete refresh
	class BlankState final : public State {
	  public:
		BlankState() = default;
		bool getHeaderAndCommandOnly() const override { return true; }
		void begin(EngineClient&) override;
		void update(EngineClient&) override;

	  private:
		/// Avoid flooding the server with requests for the world.
		int mSendFrame{0};
	};

	State*			   mState;
	ClientStartedState mClientStartedState;
	RunningState	   mRunningState;
	BlankState		   mBlankState;

	void setState(State&);
};

} // namespace ds

#endif
