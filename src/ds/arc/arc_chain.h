#pragma once
#ifndef DS_ARC_ARCCHAIN_H_
#define DS_ARC_ARCCHAIN_H_

#include <vector>
#include "ds/arc/arc.h"

namespace ds {
namespace arc {

/**
 * \class ds::arc::Chain
 * \brief Store a list of arcs.
 */
class Chain : public Arc
{
public:
	Chain();
	
	virtual void				renderCircle(RenderCircleParams&) const;

	virtual void				readXml(const ci::XmlTree&);

private:
	std::vector<std::unique_ptr<Arc>>	mArc;
};

} // namespace arc
} // namespace ds

#endif // DS_ARC_ARCRENDER_H_