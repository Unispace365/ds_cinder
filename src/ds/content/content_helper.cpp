#include "stdafx.h"

#include "ds/content/content_helper.h"

namespace ds::model {

ContentHelperPtr ContentHelperFactory::mDefault = nullptr;

const std::string ContentHelper::DEFAULTCATEGORY	  = "default!";
const std::string ContentHelper::WAFFLESCATEGORY	  = "waffles!";
const std::string ContentHelper::PRESENTATIONCATEGORY = "presentation";
const std::string ContentHelper::AMBIENTCATEGORY	  = "ambient";

std::vector<ContentModelRef> ContentHelper::getRecordsOfType(const std::vector<ContentModelRef>& records,
															 const std::string&					 type) {
	auto allOfType = std::vector<ContentModelRef>();
	getRecordsByType(records, type, allOfType);
	return allOfType;
}

std::vector<ContentProperty> ContentHelper::findAllProperties(const std::vector<ContentModelRef>& records,
															  const std::string&				  propertyName) {
	auto allProps = std::vector<ContentProperty>();
	getPropertyByName(records, propertyName, allProps);
	return allProps;
}

void ContentHelper::getRecordsByUid(const std::vector<ContentModelRef>& records, const std::string& uid,
									std::vector<ContentModelRef>& result) {
	for (const auto& it : records) {
		if (it.getUid() == uid || it.getPropertyString("uid") == uid) {
			result.push_back(it);
		} else {
			getRecordsByUid(it.getChildren(), uid, result);
		}
	}
}

void ContentHelper::getRecordsByType(const std::vector<ContentModelRef>& records, const std::string& type,
									 std::vector<ContentModelRef>& result) {
	for (const auto& it : records) {
		if (it.getPropertyString("type_key") == type) {
			result.push_back(it);
		} else {
			getRecordsByType(it.getChildren(), type, result);
		}
	}
}

void ContentHelper::getPropertyByName(const std::vector<ContentModelRef>& records, const std::string& propertyName,
									  std::vector<ContentProperty>& result) {
	for (const auto& it : records) {
		auto prop = it.getProperty(propertyName);
		if (!prop.empty()) result.push_back(prop);

		const auto& children = it.getChildren();
		if (!children.empty()) getPropertyByName(children, propertyName, result);
	}
}

} // namespace ds::model
