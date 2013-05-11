#include "ds/arc/arc_input.h"

namespace ds {
namespace arc {

/**
 * ds::arc::Input
 */
Input::Input()
{
}

ci::ColorA Input::getColor(const size_t index, const ci::ColorA& defaultValue) const
{
	if (index < mColor.size()) return mColor[index];
	return defaultValue;
}

double Input::getFloat(const size_t index, const double defaultValue) const
{
	if (index < mFloat.size()) return mFloat[index];
	return defaultValue;
}

void Input::addColor(const ci::ColorA& v)
{
	mColor.push_back(v);
}

void Input::addFloat(const double v)
{
	mFloat.push_back(v);
}

/**
 * ds::arc::ColorParam
 */
ColorParam::ColorParam(const ci::ColorA& value)
	: mValue(value)
	, mInputIndex(-1)
{
}

ci::ColorA ColorParam::getValue(const Input& input) const
{
	if (mInputIndex < 0) return mValue;
	return input.getColor(static_cast<size_t>(mInputIndex), mValue);
}

void ColorParam::readXml(const ci::XmlTree& xml)
{
	mInputIndex = xml.getAttributeValue<int>("input", -1);
}

/**
 * ds::arc::FloatParam
 */
FloatParam::FloatParam(const double value)
	: mValue(value)
	, mInputIndex(-1)
{
}

double FloatParam::getValue(const Input& input) const
{
	if (mInputIndex < 0) return mValue;
	return input.getFloat(static_cast<size_t>(mInputIndex), mValue);
}

void FloatParam::readXml(const ci::XmlTree& xml)
{
	mInputIndex = xml.getAttributeValue<int>("input", -1);
}

} // namespace arc
} // namespace ds
