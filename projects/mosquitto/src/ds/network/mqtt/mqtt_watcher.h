#ifndef DS_NETWORK_MQTT_MQTT_WATCHER
#define DS_NETWORK_MQTT_MQTT_WATCHER

#include <queue>
#include <mutex>
#include <atomic>

#include <ds/app/auto_update.h>

namespace ds {
namespace net {

/**
* \class ds::MqttWatcher
* \brief Listen and send messages on MQTT ( http://mqtt.org )
*/
class MqttWatcher : public ds::AutoUpdate
{
public:
	typedef std::queue<std::string> MessageQueue;

	// Standard MQTT location
	MqttWatcher(ds::ui::SpriteEngine&,
				const std::string& host, //example: "test.mosquitto.org"
				const std::string& topic_inbound, // "/ds_test_mqtt_inbound"
				const std::string& topic_outbound, // "/ds_test_mqtt_outbound"
				float refresh_rate = 0.1f,
				int port = 1883);

	virtual ~MqttWatcher();

	void							startListening();
	void							addInboundListener(const std::function<void(const MessageQueue&)>&);
	void							sendOutboundMessage(const std::string&);
	void							setTopicInbound(const std::string&);
	void							setHostString(const std::string&);
	void							setPort(const int);

	// How long to wait to restart the connection
	// Set to a negative number to never retry on connection failure
	void							setRetryWaitTime(const float timeInSeconds){ mRetryWaitTime = timeInSeconds; }

protected:
	virtual void					update(const ds::UpdateParams &) override;

private:
	class MqttConnectionLoop {
	public:
		std::mutex					mInboundMutex;
		std::mutex					mOutboundMutex;
		std::atomic<bool>			mAbort;
		std::atomic<bool>			mConnected{ false };
		MessageQueue				mLoopInbound;
		MessageQueue				mLoopOutbound;

		MqttConnectionLoop(ds::ui::SpriteEngine&,
			 const std::string& host,
			 const std::string& topic_inbound,
			 const std::string& topic_outband,
			 float refresh_rate,
			 int port);

		virtual void				run();
		void						setInBound(const std::string&);
		void						setHost(const std::string&);
		void						setPort(const int);

	private:
		std::string					mHost;
		std::string					mTopicInbound;
		std::string					mTopicOutbound;
		int							mPort;
		const int					mRefreshRateMs;	// in milliseconds
		bool						mFirstTimeMessage;
	};

	MessageQueue					mMsgInbound;
	MessageQueue					mMsgOutbound;
	std::vector < std::function<void(const MessageQueue&)> >
		mListeners;
	MqttConnectionLoop				mLoop;

	Poco::Timestamp::TimeVal		mLastMessageTime;
	float							mRetryWaitTime;
};

} //!namespace net
} //!namespace ds

#endif
