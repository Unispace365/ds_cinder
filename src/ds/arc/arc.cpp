#include "stdafx.h"

#include "ds/arc/arc.h"

namespace ds {
namespace arc {

/**
 * ds::arc::Arc
 */
Arc::Arc()
{
}

Arc::~Arc()
{
}

double Arc::run(const Input&, const double v) const
{
	return v;
}

void Arc::renderCircle(const Input&, RenderCircleParams&) const
{
}

void Arc::readXml(const ci::XmlTree&)
{
}

} // namespace arc
} // namespace ds
