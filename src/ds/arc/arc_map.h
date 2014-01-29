#pragma once
#ifndef DS_ARC_ARCMAP_H_
#define DS_ARC_ARCMAP_H_

#include <vector>
#include "ds/arc/arc.h"
#include "ds/arc/arc_input.h"

namespace ds {
namespace arc {

/**
 * \class ds::arc::Map
 * \brief Perform a mapping between two ranges. The input is
 * first clipped to the from range, then mapped to the to range.
 * By default, my ranges are 0-1 and 0-1, meaning I clipped
 * everything to 0-1.
 */
class Map : public Arc {
public:
	Map();

	virtual double		run(const Input&, const double) const;

	virtual void		readXml(const ci::XmlTree&);

private:
	FloatParam			mFromMin, mFromMax,
						mToMin, mToMax;
};

} // namespace arc
} // namespace ds

#endif // DS_ARC_ARCMAP_H_