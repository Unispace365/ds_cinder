#pragma once
#ifndef DS_ARC_ARCINPUT_H_
#define DS_ARC_ARCINPUT_H_

#include <vector>
#include <cinder/Color.h>
#include <cinder/Xml.h>

namespace ds {
namespace arc {

/**
 * \class ds::arc::Input
 * \brief Provide input to an arc from an external source.
 */
class Input
{
public:
	Input();

	ci::ColorA				getColor(const size_t index, const ci::ColorA& defaultValue) const;
	double					getFloat(const size_t index, const double defaultValue) const;

	void					addColor(const ci::ColorA&);
	void					addFloat(const double);

private:
	std::vector<ci::ColorA>	mColor;
	std::vector<double>		mFloat;
};

/**
 * \class ds::arc::ColorParam
 * \brief A standard color, but one with the potential to be replaced
 * by a value from an Input.
 */
class ColorParam
{
public:
	ColorParam(const ci::ColorA& clr = ci::ColorA(0.0f, 0.0f, 0.0f, 1.0f));

	ci::ColorA		getValue(const Input&) const;

	void			readXml(const ci::XmlTree&);

	ci::ColorA		mValue;
	int				mInputIndex;
};

/**
 * \class ds::arc::FloatParam
 * \brief A standard float, but one with the potential to be replaced
 * by a value from an Input.
 */
class FloatParam
{
public:
	FloatParam(const double value = 0.0);

	double			getValue(const Input&) const;

	void			readXml(const ci::XmlTree&);

	double			mValue;
	int				mInputIndex;
};

} // namespace arc
} // namespace ds

#endif // DS_ARC_ARCINPUT_H_