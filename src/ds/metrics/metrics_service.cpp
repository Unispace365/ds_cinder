#include "stdafx.h"

#include "metrics_service.h"

#include <ds/debug/computer_info.h>
#include <ds/debug/logger.h>
#include <ds/util/string_util.h>


namespace ds {

MetricsService::MetricsService(ds::Engine& eng)
  : mEngine(eng)
  , mActive(true)
  , mSendBaseInfo(true)
  , mSendTouchInfo(false)
  , mCallbacks(eng) {

	mActive		   = mEngine.getEngineSettings().getBool("metrics:active");
	mSendBaseInfo  = mEngine.getEngineSettings().getBool("metrics:send_base_info");
	mSendTouchInfo = mEngine.getEngineSettings().getBool("metrics:send_touch_info");

	if (mActive) {
		std::string host = mEngine.getEngineSettings().getString("metrics:udp_host");
		std::string port = mEngine.getEngineSettings().getString("metrics:udp_port");

		if (mUdpReccy.connect(host, port)) {
			DS_LOG_INFO("MetricsService: connected to telegraf udp at " << host << ":" << port);
		}

		if (mSendBaseInfo) {
			double callbackDelay = mEngine.getEngineSettings().getDouble("metrics:base_info_send_delay");
			if (callbackDelay < 0.1)
				callbackDelay = 0.1; // don't swamp it with too much info (this is probably still too much)
			mCallbacks.repeatedCallback([this] { sendSystemInfo(); }, callbackDelay);
		} else {
			mCallbacks.cancel();
		}
	}
}

void MetricsService::sendSystemInfo() {
	auto fps = mEngine.getAverageFps();
	recordMetric("engine", "fps", std::to_string(fps));
	recordMetric("engine", "sprites", std::to_string(mEngine.getNumberOfSprites()));
	recordMetric("engine", "physical_memory",
				 std::to_string(mEngine.getComputerInfo().getPhysicalMemoryUsedByProcess()));
	recordMetric("engine", "virtual_memory", std::to_string(mEngine.getComputerInfo().getVirtualMemoryUsedByProcess()));
}

void MetricsService::sanitizeString(std::string& theStr) {
	ds::replace(theStr, " ", "");
	ds::replace(theStr, "\\", "");
}

void MetricsService::recordMetric(const std::string& metricName, const std::string& fieldName,
								  const std::string& fieldValue) {
	std::string appName		  = mEngine.getEngineData().mAppInstanceName;
	auto		theMetricName = metricName;
	auto		theFieldName  = fieldName;
	auto		theFieldValue = fieldValue;
	sanitizeString(appName);
	sanitizeString(theMetricName);
	sanitizeString(theFieldName);
	sanitizeString(theFieldValue);
	sendMetrics(theMetricName + ",app=" + appName + " " + theFieldName + "=" + theFieldValue + "\n");
}

void MetricsService::recordMetric(const std::string& metricName, const std::string& fieldName, const int& fieldValue) {
	recordMetric(metricName, fieldName, std::to_string(fieldValue));
}

void MetricsService::recordMetric(const std::string& metricName, const std::string& fieldName,
								  const float& fieldValue) {
	recordMetric(metricName, fieldName, std::to_string(fieldValue));
}

void MetricsService::recordMetric(const std::string& metricName, const std::string& fieldName,
								  const double& fieldValue) {
	recordMetric(metricName, fieldName, std::to_string(fieldValue));
}

void MetricsService::recordMetric(const std::string& metricName, const std::string& fieldName,
								  const ci::vec2& fieldValue) {
	recordMetric(metricName, fieldName + "_x", std::to_string(fieldValue.x));
	recordMetric(metricName, fieldName + "_y", std::to_string(fieldValue.y));
}

void MetricsService::recordMetric(const std::string& metricName, const std::string& fieldName,
								  const ci::vec3& fieldValue) {
	recordMetric(metricName, fieldName + "_x", std::to_string(fieldValue.x));
	recordMetric(metricName, fieldName + "_y", std::to_string(fieldValue.y));
	recordMetric(metricName, fieldName + "_z", std::to_string(fieldValue.z));
}

void MetricsService::recordMetric(const std::string& metricName, const std::string& fieldName,
								  const ci::Rectf& fieldValue) {
	recordMetric(metricName, fieldName + "_x", std::to_string(fieldValue.x1));
	recordMetric(metricName, fieldName + "_y", std::to_string(fieldValue.y1));
	recordMetric(metricName, fieldName + "_w", std::to_string(fieldValue.getWidth()));
	recordMetric(metricName, fieldName + "_h", std::to_string(fieldValue.getHeight()));
}

void MetricsService::recordMetric(const std::string& metricName, const std::string& fieldNameAndValue) {
	std::string appName		  = mEngine.getEngineData().mAppInstanceName;
	auto		theMetricName = metricName;
	auto		theFieldValue = fieldNameAndValue;
	sanitizeString(theMetricName);
	sanitizeString(theFieldValue);
	sendMetrics(theMetricName + ",app=" + appName + " " + fieldNameAndValue + "\n");
}

void MetricsService::recordMetricString(const std::string& metricName, const std::string& fieldName,
										const std::string& stringValue) {
	recordMetric(metricName, fieldName, "\"" + stringValue + "\"");
}

void MetricsService::recordMetricString(const std::string& metricName, const std::string& fieldName,
										const std::wstring& stringValue) {
	recordMetricString(metricName, fieldName, ds::utf8_from_wstr(stringValue));
}

void MetricsService::recordMetricTouch(ds::ui::TouchInfo& ti) {
	if (!mSendTouchInfo) return;

	// ignore moved.
	if (ti.mPhase == ds::ui::TouchInfo::Moved) return;

	/// Save a separate setting so we can track number of inputs separate from all of the info
	if (ti.mPhase == ds::ui::TouchInfo::Added) {
		recordMetric("input", "added", "1");
	} else if (ti.mPhase == ds::ui::TouchInfo::Removed) {
		recordMetric("input", "removed", "1");
	}
	recordMetric("input", "phase", ti.mPhase);
	recordMetric("input", "finger_id", ti.mFingerId);
	recordMetric("input", "pos", ti.mCurrentGlobalPoint);
}

void MetricsService::sendMetrics(const std::string& metrix) {
	DS_LOG_VERBOSE(3, metrix);
	if (mUdpReccy.isConnected()) {
		mUdpReccy.sendMessage(metrix);
	}
}

} // namespace ds
