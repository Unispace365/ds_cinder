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
	/// Auto-tags all recordings with the app name and sanitizes the output
	MetricsService(ds::Engine&);

	/// Sends the raw field value, so if this should be a string to be saved, you'll need to wrap it in quotes or use the recordMetricString() function
	void								recordMetric(const std::string& metricName, const std::string& fieldName, const std::string& fieldValue);
	void								recordMetric(const std::string& metricName, const std::string& fieldName, const int& fieldValue);
	void								recordMetric(const std::string& metricName, const std::string& fieldName, const float& fieldValue);
	void								recordMetric(const std::string& metricName, const std::string& fieldName, const double& fieldValue);
	/// Appends _x and _y to field name to save 2 metrics, one for each part of the vector
	void								recordMetric(const std::string& metricName, const std::string& fieldName, const ci::vec2& fieldValue);
	/// Appends _x, _y and _z to field name to save 3 metrics, one for each part of the vector
	void								recordMetric(const std::string& metricName, const std::string& fieldName, const ci::vec3& fieldValue);
	/// Appends _x, _y, _w, _h to field name to save 4 metrics, one for each part of the rect
	void								recordMetric(const std::string& metricName, const std::string& fieldName, const ci::Rectf& fieldValue);

	/// Combined field and value in the format field0=fieldValue,field1=field1value 
	/// Use this if you're sending multiple fields that should have the same timestamp
	/// You'll need to wrap any string values in quotes
	void								recordMetric(const std::string& metricName, const std::string& fieldNameAndValue);

	/// Wraps the value in quotes
	void								recordMetricString(const std::string& metricName, const std::string& fieldName, const std::string& stringValue);
	void								recordMetricString(const std::string& metricName, const std::string& fieldName, const std::wstring& stringValue);

	/// Saves an input with x, y, fingerid and phase
	void								recordMetricTouch(ds::ui::TouchInfo& ti);



private:
	void								sendMetrics(const std::string& metrix);
	void								sendSystemInfo();

	bool								mActive;
	bool								mSendBaseInfo;
	bool								mSendTouchInfo;
	
	ds::time::Callback					mCallbacks;
	ds::Engine&							mEngine;
	ds::UdpReceiver						mUdpReccy;

	void								sanitizeString(std::string& theStr);
};

} 

#endif // !DS_METRICS_METRICS_SERVICE_
