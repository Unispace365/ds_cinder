#pragma once
#ifndef DS_NETWORK_TCPCLIENT_H_
#define DS_NETWORK_TCPCLIENT_H_

#include "ds/app/auto_update.h"
#include <Poco/Condition.h>
#include <Poco/Mutex.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Runnable.h>
#include <Poco/Thread.h>
#include <functional>
#include <vector>

namespace ds { namespace net {

	/**
	 * \class TcpClient
	 * \brief Feed clients information about changes on a TCP socket.
	 */
	class TcpClient : public ds::AutoUpdate {
	  public:
		class Options {
		  public:
			explicit Options(const double poll_rate = 1.0, const int receive_buffer_size = 0,
							 const int send_buffer_size = 0);

			double mPollRate;
			int	   mReceiveBufferSize;
			int	   mSendBufferSize;
		};

	  public:
		/// Utility function shared between client and server
		static void sendBytes(Poco::Net::StreamSocket&, const std::string& data);

		/// If there's a terminator character, then I will split input by the terminator,
		/// and hold onto anything that's missing it.
		TcpClient(ds::ui::SpriteEngine&, const Poco::Net::SocketAddress&, const Options& opt = Options(),
				  const std::string& terminator = "");
		~TcpClient();

		void add(const std::function<void(const std::string&)>&);

		/// Send data to the server
		void send(const std::string& data);

	  protected:
		/// Flush any change notifications from the calling thread.
		virtual void update(const ds::UpdateParams&);

	  private:
		class Loop : public Poco::Runnable {
		  public:
			Poco::Mutex				 mMutex;
			bool					 mAbort;
			std::vector<std::string> mSendData;
			std::vector<std::string> mUpdates;
			Poco::Net::StreamSocket	 mSocket;

		  public:
			Loop(const Poco::Net::SocketAddress&, const Options&, const std::string& terminator);

			virtual void run();

		  private:
			void sendTo(Poco::Net::StreamSocket&);
			void update(const std::string&);

			const Poco::Net::SocketAddress mAddress;
			const Options				   mOptions;
			/// If there's a terminator char, I'll hold onto any incoming
			/// data without it.
			const std::string mTerminator;
			std::string		  mWaiting;
		};

		Poco::Thread										 mThread;
		Loop												 mLoop;
		std::vector<std::function<void(const std::string&)>> mListener;
	};

}} // namespace ds::net

#endif // DS_NETWORK_TCPCLIENT_H_
