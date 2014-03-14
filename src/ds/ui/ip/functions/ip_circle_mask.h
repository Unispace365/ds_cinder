#pragma once
#ifndef DS_UI_IP_FUNCTIONS_IPCIRCLEMASK_H_
#define DS_UI_IP_FUNCTIONS_IPCIRCLEMASK_H_

#include <ds/ui/ip/ip_function.h>

namespace ds {
namespace ui {
namespace ip {

/**
 * \class ds::ui::ip::CircleMask
 * Make the surface circular by alphing-out anything outside the circle.
 */
class CircleMask : public Function {
public:
	CircleMask();
		
	virtual void				on(const std::string& parameters, ci::Surface8u&) const;
};

} // namespace ip
} // namespace ui
} // namespace ds

#endif