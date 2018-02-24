#pragma once
#ifndef DS_METRICS_METRICS_SERVICE_
#define DS_METRICS_METRICS_SERVICE_

#include <ds/app/engine/engine.h>
#include <ds/network/single_udp_receiver.h>
#include <ds/app/auto_update.h>

namespace ds {

/**
* \class vm::MetricsService
* \brief Sends metrics data to a telegraf data collector
*/
class MetricsService {
public:
	MetricsService(ds::Engine&);

	/// Auto-tags with the app name
	void								recordMetric(const std::string& metricName, const std::string& fieldName, const std::string& fieldValue);

private:
	void								sendMetrics(const std::string& metrix);
	void								sendSystemInfo();

	bool								mActive;
	bool								mSendBaseInfo;
	
	ds::time::Callback					mCallbacks;
	ds::Engine&							mEngine;
	ds::UdpReceiver						mUdpReccy;

	void								sanitizeString(std::string& theStr);
};

} 

#endif // !DS_METRICS_METRICS_SERVICE_
