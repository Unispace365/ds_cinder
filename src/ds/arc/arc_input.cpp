#include "ds/arc/arc_input.h"

#include <cinder/Xml.h>
#include "ds/data/data_buffer.h"

namespace ds {
namespace arc {

namespace {
const size_t		FLOAT_MIN_F = (1<<0);
const size_t		FLOAT_MAX_F = (1<<1);

const char			COLOR_ATT		= 20;
const char			FLOAT_ATT		= 21;
const char			VEC2_ATT		= 22;
const char			END_ATT			= 0;
}

/**
 * ds::arc::Input
 */
Input::Input() {
}

ci::ColorA Input::getColor(const size_t index, const ci::ColorA& defaultValue) const {
	if (index < mColor.size()) return mColor[index];
	return defaultValue;
}

double Input::getFloat(const size_t index, const double defaultValue) const {
	if (index < mFloat.size()) return mFloat[index];
	return defaultValue;
}

ci::Vec2d Input::getVec2(const size_t index, const ci::Vec2d& defaultValue) const {
	if (index < mVec2.size()) return mVec2[index];
	return defaultValue;
}

void Input::addColor(const ci::ColorA& v) {
	mColor.push_back(v);
}

void Input::addFloat(const double v) {
	mFloat.push_back(v);
}

void Input::addVec2(const ci::Vec2d& v) {
	mVec2.push_back(v);
}

void Input::writeTo(DataBuffer& buf) const {
	for (auto it=mColor.begin(), end=mColor.end(); it!=end; ++it) {
		buf.add(COLOR_ATT);
		buf.add(*it);
	}
	for (auto it=mFloat.begin(), end=mFloat.end(); it!=end; ++it) {
		buf.add(FLOAT_ATT);
		buf.add(*it);
	}
	for (auto it=mVec2.begin(), end=mVec2.end(); it!=end; ++it) {
		buf.add(VEC2_ATT);
		buf.add(*it);
	}
	buf.add(END_ATT);
}

bool Input::readFrom(DataBuffer& buf) {
	try {
		while (buf.canRead<char>()) {
			const char		att = buf.read<char>();
			if (att == COLOR_ATT) {
				mColor.push_back(buf.read<ci::ColorA>());
			} else if (att == FLOAT_ATT) {
				mFloat.push_back(buf.read<double>());
			} else if (att == VEC2_ATT) {
				mVec2.push_back(buf.read<ci::Vec2d>());
			} else if (att == END_ATT) {
				return true;
			} else {
				return false;
			}
		}
	} catch (std::exception const&) {
	}
	return false;
}

/**
 * ds::arc::ColorParam
 */
ColorParam::ColorParam(const ci::ColorA& value)
		: mValue(value)
		, mInputIndex(-1) {
}

ci::ColorA ColorParam::getValue(const Input& input) const {
	if (mInputIndex < 0) return mValue;
	return input.getColor(static_cast<size_t>(mInputIndex), mValue);
}

void ColorParam::readXml(const ci::XmlTree& xml) {
	mInputIndex = xml.getAttributeValue<int>("input", -1);
}

/**
 * ds::arc::FloatParam
 */
FloatParam::FloatParam(const double value)
		: mValue(value)
		, mInputIndex(-1)
		, mFlags(0)
		, mMin(value)
		, mMax(value) {
}

double FloatParam::getValue(const Input& input) const {
	double			ans = mValue;
	if (mInputIndex >= 0) {
		ans = input.getFloat(static_cast<size_t>(mInputIndex), mValue);
	}
	if ((mFlags&FLOAT_MIN_F) != 0 && ans < mMin) ans = mMin;
	if ((mFlags&FLOAT_MAX_F) != 0 && ans > mMax) ans = mMax;
	return ans;
}

void FloatParam::readXml(const ci::XmlTree& xml) {
	mInputIndex = xml.getAttributeValue<int>("input", -1);
	size_t			flags = 0;
	if (xml.hasAttribute("min")) {
		mMin = xml.getAttributeValue<double>("min", mMin);
		flags |= FLOAT_MIN_F;
	}
	if (xml.hasAttribute("max")) {
		mMax = xml.getAttributeValue<double>("max", mMax);
		flags |= FLOAT_MAX_F;
	}
	mFlags = flags;
}

/**
 * ds::arc::Vec2Param
 */
Vec2Param::Vec2Param(const ci::Vec2d& value)
		: mValue(value)
		, mInputIndex(-1) {
}

ci::Vec2d Vec2Param::getValue(const Input& input) const {
	if (mInputIndex < 0) return mValue;
	return input.getVec2(static_cast<size_t>(mInputIndex), mValue);
}

void Vec2Param::readXml(const ci::XmlTree& xml) {
	mInputIndex = xml.getAttributeValue<int>("input", -1);
}

} // namespace arc
} // namespace ds
