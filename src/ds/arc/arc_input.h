#pragma once
#ifndef DS_ARC_ARCINPUT_H_
#define DS_ARC_ARCINPUT_H_

#include <vector>
#include <cinder/Color.h>
#include <cinder/Vector.h>

namespace cinder {
class XmlTree;
}

namespace ds {
class DataBuffer;

namespace arc {

/**
 * \class ds::arc::Input
 * \brief Provide input to an arc from an external source.
 */
class Input {
public:
	Input();

	ci::ColorA				getColor(const size_t index, const ci::ColorA& defaultValue) const;
	double					getFloat(const size_t index, const double defaultValue) const;
	ci::dvec2				getVec2(const size_t index, const ci::dvec2& defaultValue) const;

	void					addColor(const ci::ColorA&);
	void					addFloat(const double);
	void					addVec2(const ci::dvec2&);

	void					writeTo(DataBuffer&) const;
	bool					readFrom(DataBuffer&);

private:
	std::vector<ci::ColorA>	mColor;
	std::vector<double>		mFloat;
	std::vector<ci::dvec2>	mVec2;
};

/**
 * \class ds::arc::ColorParam
 * \brief A standard color, but one with the potential to be replaced
 * by a value from an Input.
 */
class ColorParam {
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
class FloatParam {
public:
	FloatParam(const double value = 0.0);

	double			getValue(const Input&) const;

	void			readXml(const ci::XmlTree&);

	double			mValue;
	int				mInputIndex;

private:
	std::size_t		mFlags;
	double			mMin, mMax;
};

/**
 * \class ds::arc::Vec2Param
 * \brief A standard Vec2, but one with the potential to be replaced
 * by a value from an Input.
 */
class Vec2Param {
public:
	Vec2Param(const ci::dvec2& v = ci::dvec2(0.0f, 0.0f));

	ci::dvec2		getValue(const Input&) const;

	void			readXml(const ci::XmlTree&);

	ci::dvec2		mValue;
	int				mInputIndex;
};

} // namespace arc
} // namespace ds

#endif // DS_ARC_ARCINPUT_H_