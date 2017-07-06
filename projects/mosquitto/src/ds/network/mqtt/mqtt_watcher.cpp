#include "mqtt_watcher.h"

#include "ds/network/mosquitto/mosquittopp.h"

#include <ds/cfg/settings.h>
#include <ds/debug/debug_defines.h>
#include <ds/ui/sprite/sprite_engine.h>
#include "ds/debug/logger.h"

namespace ds {
namespace net {

namespace {

//for initialization and convenience
class MqttSingleton {
public:
	void static initilize_once() { static MqttSingleton S; }
	~MqttSingleton() { mosqpp::lib_cleanup(); }

private:
	MqttSingleton() { mosqpp::lib_init(); };
};

const static ds::BitMask   MQTT_LOG = ds::Logger::newModule("mqtt");

static void _clear_queue(MqttWatcher::MessageQueue& q){
	if(!q.empty()) MqttWatcher::MessageQueue().swap(q);
}
} //!namespace


MqttWatcher::MqttWatcher(
	ds::ui::SpriteEngine& e,
	const std::string& host /*= "test.mosquitto.org"*/,
	const std::string& topic_inbound /*= "/ds_test_mqtt_inbound"*/,
	const std::string& topic_outband /*= "/ds_test_mqtt_outbound"*/,
	float refresh_rate /*= 0.1f*/,
	int port /*= 1883*/,
	const std::string& clientId)
	: ds::AutoUpdate(e)
	, mLoop(e, host, topic_inbound, topic_outband, refresh_rate, port, clientId)
	, mRetryWaitTime(5.0f)
	, mStarted(false)
{
	MqttSingleton::initilize_once();
	mLastMessageTime = Poco::Timestamp().epochMicroseconds();
}

MqttWatcher::~MqttWatcher(){
	stopListening();
}

void MqttWatcher::addInboundListener(const std::function<void(const MessageQueue&)>& fn){
	if(!fn) return;
	mListeners.push_back(fn);
}

void MqttWatcher::update(const ds::UpdateParams &){
	if(mStarted && !mLoop.mConnected && mRetryWaitTime > 0.0f){
		Poco::Timestamp::TimeVal nowwy = Poco::Timestamp().epochMicroseconds();
		auto delty = (float)(nowwy - mLastMessageTime) / 1000000.0f;
		if(delty > mRetryWaitTime){
			startListening();
			mLastMessageTime = Poco::Timestamp().epochMicroseconds();
		}
	}

	_clear_queue(mMsgInbound);

	if(!mLoop.mLoopInbound.empty()){
		std::lock_guard<std::mutex>	_lock(mLoop.mInboundMutex);
		//double check
		if(!mLoop.mLoopInbound.empty()){
			mLoop.mLoopInbound.swap(mMsgInbound);
		}
	}

	if(!mMsgOutbound.empty()){
		std::lock_guard<std::mutex>	_lock(mLoop.mOutboundMutex);
		while(!mMsgOutbound.empty()){
			mLoop.mLoopOutbound.push_back(mMsgOutbound.back());
			mMsgOutbound.pop_back();
		}
	}

	if(!mMsgInbound.empty()){
		for(const std::function<void(const MessageQueue&)>& cb : mListeners){
			cb(mMsgInbound);
		}
	}
}

void MqttWatcher::sendOutboundMessage(const std::string& str){
	//Dont need to set topic for outbound messages. Handled through setting outbound topic
	MqttMessage outMsg;
	outMsg.message = std::string(str);
	mMsgOutbound.push_back(outMsg);
}

void MqttWatcher::startListening(){
	if(mStarted){
		stopListening();
	}
	mStarted = true;
	if(!mLoop.mConnected){
		DS_LOG_INFO_M("Attempting to connect to the MQTT server at " << mLoop.getHost() << ":" << mLoop.getPort(), MQTT_LOG);
		mLoopThread = std::thread( [this](){ mLoop.run(); } );
		mLoop.mConnected = true;
	}
}

void MqttWatcher::stopListening(){
	if(!mStarted) return;
	mStarted = false;
	// setting abort is atomic, but someone may want to do something right after this
	std::lock_guard<std::mutex>	_lock(mLoop.mOutboundMutex);
	DS_LOG_INFO_M("Closing connection to the MQTT server at " << mLoop.getHost() << ":" << mLoop.getPort(), MQTT_LOG);
	mLoop.mAbort = true;
	if(mLoopThread.joinable()){
		mLoopThread.join();
	}
}

void MqttWatcher::setTopicInbound(const std::string& inBound){
	std::lock_guard<std::mutex>	_lock(mLoop.mInboundMutex);
	mLoop.setInBound(inBound);
}

void MqttWatcher::setTopicOutbound(const std::string& outBound){
	std::lock_guard<std::mutex>	_lock(mLoop.mOutboundMutex);
	mLoop.setOutBound(outBound);
}

void MqttWatcher::setHostString(const std::string& host){
	mLoop.setHost(host);
}

void MqttWatcher::setPort(const int port){
	mLoop.setPort(port);
}

/**
* \class bmc::MqttWatcher::Loop
*/

MqttWatcher::MqttConnectionLoop::MqttConnectionLoop (
	ds::ui::SpriteEngine&,
	const std::string& host,
	const std::string& topic_inbound,
	const std::string& topic_outband,
	float refresh_rate,
	int port,
	const std::string& clientId)
	: mAbort(false)
	, mHost(host)
	, mPort(port)
	, mTopicInbound(topic_inbound)
	, mTopicOutbound(topic_outband)
	, mRefreshRateMs(static_cast<int>(refresh_rate * 1000))
	, mFirstTimeMessage(true)
	, mClientId(clientId)
{}

namespace {
class MosquittoReceiver final : public mosqpp::mosquittopp
{
public:
	MosquittoReceiver(const std::string& id) : mosqpp::mosquittopp(id.c_str(), true){}
	void on_connect(int rc) override { mConnectAction(rc); }
	void on_message(const struct mosquitto_message *message) override { 
		MqttWatcher::MqttMessage msg;
		msg.topic = std::string((char*)message->topic);
		msg.message = std::string((char*)message->payload, message->payloadlen);
		mMessageAction(msg);
	}
	void setConnectAction(const std::function<void(int)>& fn) { mConnectAction = fn; }
	void setMessageAction(const std::function<void(const MqttWatcher::MqttMessage&)>& fn) { mMessageAction = fn; }

private:
	std::function<void(int)>				mConnectAction{ [](int){} };
	std::function<void(const MqttWatcher::MqttMessage&)>	mMessageAction{ [](const MqttWatcher::MqttMessage&){} };
};
}

void MqttWatcher::MqttConnectionLoop::run(){
	mAbort = false;

	std::srand((unsigned int)std::time(0));
	std::string id = mClientId;
	if(id.empty()) id = "ds_" + std::to_string(std::time(0));
	id.append(std::to_string(std::rand()));

	MosquittoReceiver mqtt_isnt(id);

	mqtt_isnt.setConnectAction([this, id](int code){
		DS_LOG_INFO_M("MQTT server connected, status code (0 is success): " << code << " client id: " << id, MQTT_LOG);
		mFirstTimeMessage = true;
	});

	mqtt_isnt.setMessageAction([this](const MqttMessage& msg){
		//std::cout << "MQTT watcher received: " << m << std::endl, MQTT_LOG;
		if(!mAbort)	{
			std::lock_guard<std::mutex>	_lock(mInboundMutex);
			//PUSHES BOTH THE TOPIC AND THE PAYLOAD PER MESSAGE
			mLoopInbound.push_back(msg);
		}
	});

	auto err_no = mqtt_isnt.connect(mHost.c_str(), mPort);
	if(err_no != MOSQ_ERR_SUCCESS && mFirstTimeMessage){
		DS_LOG_ERROR_M("Unable to connect to the MQTT server. Error number is: " << err_no << ". Error string is: " << mosqpp::strerror(err_no), MQTT_LOG);
		mAbort = true;
	}

	err_no = mqtt_isnt.subscribe(nullptr, mTopicInbound.c_str());
	if(err_no != MOSQ_ERR_SUCCESS && mFirstTimeMessage){
		DS_LOG_ERROR_M("Unable to subscribe to the MQTT topic (" << mTopicInbound << "). Error number is: " << err_no << ". Error string is: " << mosqpp::strerror(err_no), MQTT_LOG);
		mAbort = true;
	}

	while(!mAbort && mqtt_isnt.loop() == MOSQ_ERR_SUCCESS){
		if(!mLoopOutbound.empty())	{
			std::lock_guard<std::mutex>	_lock(mOutboundMutex);
			while(!mLoopOutbound.empty()){
				mqtt_isnt.publish(nullptr, mTopicOutbound.c_str(), mLoopOutbound.back().message.size(), mLoopOutbound.back().message.data(), 1);
				mLoopOutbound.pop_back();
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(mRefreshRateMs));
	}

	mConnected = false;
	//if(mFirstTimeMessage) std::cout << "MQTT watcher returned.";
	mFirstTimeMessage = false;
}

void MqttWatcher::MqttConnectionLoop::setInBound(const std::string& inBound){
	mTopicInbound = inBound;
}

void MqttWatcher::MqttConnectionLoop::setOutBound(const std::string& outBound){
	mTopicOutbound = outBound;
}

void MqttWatcher::MqttConnectionLoop::setHost(const std::string& host){
	mHost.clear();
	mHost = host;
}

void MqttWatcher::MqttConnectionLoop::setPort(const int port){
	mPort = port;
}

} //!net
} //!ds
