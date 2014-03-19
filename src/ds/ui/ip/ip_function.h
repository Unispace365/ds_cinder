#pragma once
#ifndef DS_UI_IP_IPFUNCTION_H_
#define DS_UI_IP_IPFUNCTION_H_

#include <cinder/Surface.h>

namespace ds {
namespace ui {
namespace ip {

/**
 * \class ds::ui::ip::Function
 * Abstract interface for a generic image processing functions.
 */
class Function {
public:
	virtual ~Function();
	
	// Parameters can be anything. It's up to the application to
	// decide an appropriate format for this function.
	virtual void				on(const std::string& parameters, ci::Surface8u&) const = 0;

protected:
	Function();
};

/**
 * \class ds::ui::ip::FunctionRef
 * Store an IP function. The function might be run in any thread, so
 * make no assumptions. Keep the on() function reentrant and thread safe.
 */
class FunctionRef {
public:
	FunctionRef();
	explicit FunctionRef(const std::shared_ptr<Function>&);
	// Must supply a raw, unmanaged pointer (and you are relinquishing ownership).
	explicit FunctionRef(Function*);
	
	bool						empty() const;
	void						clear();

	// Parameters can be anything. It's up to the application to
	// decide an appropriate format for this function.
	void						on(const std::string& parameters, ci::Surface8u&) const;

private:
	std::shared_ptr<Function>	mFn;
};

} // namespace ip
} // namespace ui
} // namespace ds

#endif