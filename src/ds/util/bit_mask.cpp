#include "ds/util/bit_mask.h"

#include <assert.h>

using namespace ds;

class ds::BitMaskInitializer {
public:
	static BitMask		MakeClear() {
		// By definition a new state should be the empty state
		BitMask			s;
		assert(s.isEmpty());
		return s;
	}

	static BitMask		MakeFill() {
		BitMask			s;
		s.mMask = 0;
		for (int k=0; k<64; ++k) s.mMask |= (1ULL << static_cast<const uint64_t>(k));
		return s;
	}
};

namespace {
const BitMask			CLEAR = ds::BitMaskInitializer::MakeClear();
const BitMask			FILL = ds::BitMaskInitializer::MakeFill();
}

/**
 * ds::BitMask static
 */
BitMask BitMask::newEmpty()
{
	return CLEAR;
}

BitMask BitMask::newFilled()
{
	return FILL;
}

/**
 * ds::BitMask
 */
BitMask::BitMask()
	: mMask(0)
{
}

BitMask::BitMask(const int index)
	: mMask(1ULL << static_cast<const uint64_t>(index))
{
	// Currently constrained to a single 64 bit int, if we ever spill over
	// that we'll have to expand this class.
	assert(index >= 0 && index < 64);
}

bool BitMask::operator!=(const BitMask& o) const
{
  return mMask != o.mMask;
}

void BitMask::operator|=(const BitMask& o)
{
	mMask |= o.mMask;
}

BitMask BitMask::operator|(const BitMask& o) const
{
	BitMask			ans(*this);
	ans.mMask |= o.mMask;
	return ans;
}

void BitMask::operator&=(const BitMask& o)
{
	mMask &= o.mMask;
}

bool BitMask::operator&(const BitMask& o) const
{
	return (mMask&o.mMask) != 0;
}

BitMask BitMask::operator~() const
{
	BitMask			ans(*this);
	ans.mMask = ~(ans.mMask);
	return ans;
}

BitMask ds::BitMask::operator ^( const BitMask &rhs ) const
{
    BitMask temp(*this);
    temp ^= rhs;
    return temp;
}

uint64_t ds::BitMask::getMaskValue() const
{
    return mMask;
}

BitMask &BitMask::operator ^=( const BitMask &rhs )
{
    mMask ^= rhs.mMask;
    return *this;
}

bool BitMask::has(const BitMask& o) const
{
	return (mMask&o.mMask) != 0;
}

bool BitMask::isEmpty() const
{
	return mMask == 0ULL;
}

bool BitMask::isDirty() const
{
	return mMask != 0ULL;
}

void BitMask::clear()
{
	*this = CLEAR;
}

void BitMask::fill()
{
	*this = FILL;
}

int BitMask::getFirstIndex() const
{
	for (uint64_t k=0; k<64; ++k) {
		if ((mMask&(1ULL<<k)) != 0) return static_cast<int>(k);
	}
	return -1;
}
