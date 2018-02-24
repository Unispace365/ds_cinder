#include "stdafx.h"

#include "metrics_service.h"

#include <ds/debug/logger.h>
#include <ds/debug/computer_info.h>


namespace ds {

MetricsService::MetricsService(ds::Engine& eng)
	: mEngine(eng)
	, mActive(true)
	, mSendBaseInfo(true)
	, mCallbacks(eng)
{

	mActive = mEngine.getSettings("engine").getBool("metrics:active");
	mSendBaseInfo = mEngine.getSettings("engine").getBool("metrics:send_base_info");

	if(mActive) {
		std::string host = mEngine.getSettings("engine").getString("metrics:udp_host");
		std::string port = mEngine.getSettings("engine").getString("metrics:udp_port");
		if(mUdpReccy.connect(host, port)) {
			DS_LOG_INFO("MetricsService: connected to telegraf udp at " << host << ":" << port);
		}
	}

	if(mSendBaseInfo) {
		double callbackDelay = mEngine.getSettings("engine").getDouble("metrics:base_info_send_delay");
		if(callbackDelay < 0.1) callbackDelay = 0.1; // don't swamp it with too much info (this is probably still too much)
		mCallbacks.repeatedCallback([this] {
			sendSystemInfo();
		}, callbackDelay);
	} else {
		mCallbacks.cancel();
	}
}

void MetricsService::sendSystemInfo() {
	auto fps = mEngine.getAverageFps();
	recordMetric("engine", "fps", std::to_string(fps));
	recordMetric("engine", "sprites", std::to_string(mEngine.getNumberOfSprites()));
	recordMetric("engine", "physical_memory", std::to_string(mEngine.getComputerInfo().getPhysicalMemoryUsedByProcess()));
	recordMetric("engine", "virtual_memory", std::to_string(mEngine.getComputerInfo().getVirtualMemoryUsedByProcess()));
}

void MetricsService::sanitizeString(std::string& theStr) {
	ds::replace(theStr, " ", "");
	ds::replace(theStr, "\\", "");
}

void MetricsService::recordMetric(const std::string& metricName, const std::string& fieldName, const std::string& fieldValue) {
	std::string appName = mEngine.getEngineData().mAppInstanceName;
	auto theMetricName = metricName;
	auto theFieldName = fieldName;
	auto theFieldValue = fieldValue;
	sanitizeString(appName);
	sanitizeString(theMetricName);
	sanitizeString(theFieldName);
	sanitizeString(theFieldValue);
	sendMetrics(theMetricName + ",app=" + appName + " " + theFieldName + "=" + theFieldValue + "\n");
}

void MetricsService::sendMetrics(const std::string& metrix) {
	if(mUdpReccy.isConnected()) {
		mUdpReccy.sendMessage(metrix);
	}
}

}

