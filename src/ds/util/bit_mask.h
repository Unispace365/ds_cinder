#pragma once
#ifndef DS_UTIL_BITMASK_H_
#define DS_UTIL_BITMASK_H_

// Solely for the uint64_t
#include <cstdint>

namespace ds {
class BitMaskInitializer;

/**
 * \class BitMask
 * \brief Store an array of bits.
 */
class BitMask {
  public:
    static BitMask			newEmpty();
    static BitMask			newFilled();

  public:
    /**
     * \brief Create a new, empty dirty state.
     */
    BitMask();
    /**
     * \brief Create a dirty state on a single index, i.e. 0 or 45.
     */
    BitMask(const int index);

    bool          operator!= (const BitMask&) const;
    void          operator|= (const BitMask&);
    BitMask       operator| (const BitMask&) const;
    void          operator&= (const BitMask& o);
    bool          operator& (const BitMask& o) const;
    BitMask       operator~ () const;
    BitMask       operator^ (const BitMask &rhs) const;
    BitMask&      operator^= (const BitMask &rhs);

    /**
     * \brief Check to see if I have the dirty state
     */
    bool          has(const BitMask&) const;

    bool          isEmpty() const;
    /**
     * \brief Answer true if any dirty state is set
     */
    bool          isDirty() const;

    void          clear();
    /**
     * \brief Mark every possible state as dirty
     */
    void          fill();
    /**
     * \brief Answer the index of the first bit.  Index starts at 0, answer < 0 for empty.
     */
    int           getFirstIndex() const;

    /**
     * \brief returns the value of mMask. For debug purposes ONLY.
     */
    uint64_t      getMaskValue() const;

  private:
    friend class ds::BitMaskInitializer;

    /**
     * Dirty states are a bit mask.  Currently limited to 64 since that's large
     * enough for now, but if we go over we'll need to expand.
     */
    uint64_t      mMask;
};

} // namespace ds

#endif // DS_UTIL_BITMASK_H_
