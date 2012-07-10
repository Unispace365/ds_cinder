#pragma once
#ifndef DS_UTIL_MEMORYDS_H_
#define DS_UTIL_MEMORYDS_H_

#include <memory>

namespace ds {

/**
 * A safe dynamic cast between unique ptr types.
 */
template <typename Dst, typename Src>
std::unique_ptr<Dst> unique_dynamic_cast(std::unique_ptr<Src>& ptr) {
	Src						*p = ptr.release();
	std::unique_ptr<Dst>	r(dynamic_cast<Dst*>(p));
	if (!r) {
		ptr.reset(p);
	}
	return r;
}

} // namespace ds

#endif // DS_UTIL_MEMORYDS_H_
