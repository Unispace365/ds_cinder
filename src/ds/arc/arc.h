#pragma once
#ifndef DS_ARC_ARC_H_
#define DS_ARC_ARC_H_

#include <string>
#include <cinder/Xml.h>

namespace ds {
namespace arc {
class Input;
class RenderCircleParams;

/**
 * \class ds::arc::Arc
 * \brief Abstract superclass for arcs.
 */
class Arc
{
public:
	virtual ~Arc();

	// Basic perform operation. Translate the value based on subclass params
	virtual double				run(const Input&, const double) const;

	virtual void				renderCircle(const Input&, RenderCircleParams&) const;

	virtual void				readXml(const ci::XmlTree&);

protected:
	Arc();
};

} // namespace arc
} // namespace ds

#endif // DS_ARC_ARC_H_