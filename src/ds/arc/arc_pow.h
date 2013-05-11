#pragma once
#ifndef DS_ARC_ARCPOW_H_
#define DS_ARC_ARCPOW_H_

#include <vector>
#include "ds/arc/arc.h"
#include "ds/arc/arc_input.h"

namespace ds {
namespace arc {

/**
 * \class ds::arc::Pow
 * \brief Perform a pow() operation.
 */
class Pow : public Arc
{
public:
	Pow();

	virtual double		run(const Input&, const double) const;

	virtual void		readXml(const ci::XmlTree&);

private:
	FloatParam			mExp;
};

} // namespace arc
} // namespace ds

#endif // DS_ARC_ARCRENDER_H_