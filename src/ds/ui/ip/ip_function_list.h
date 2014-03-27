#pragma once
#ifndef DS_UI_IP_IPFUNCTIONLIST_H_
#define DS_UI_IP_IPFUNCTIONLIST_H_

#include <unordered_map>
#include "ip_function.h"

namespace ds {
namespace ui {
namespace ip {

/**
 * \class ds::ui::ip::FunctionList
 */
class FunctionList {
public:
	FunctionList();

	FunctionRef			find(const std::string& key) const;

	void				add(const std::string& key, const FunctionRef&);

private:
	std::unordered_map<std::string, FunctionRef>
						mFunctions;
};

} // namespace ip
} // namespace ui
} // namespace ds

#endif