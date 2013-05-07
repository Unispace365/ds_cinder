#pragma once
#ifndef DS_ARC_ARC_H_
#define DS_ARC_ARC_H_

#include <string>
#include <cinder/Xml.h>

namespace ds {
namespace arc {
class RenderCircleParams;

/**
 * \class ds::arc::Arc
 * \brief Abstract superclass for arcs.
 */
class Arc
{
public:
	virtual ~Arc();

	virtual void				renderCircle(RenderCircleParams&) const;

	virtual void				readXml(const ci::XmlTree&);

protected:
	Arc();
};

} // namespace arc
} // namespace ds

#endif // DS_ARC_ARCRENDER_H_