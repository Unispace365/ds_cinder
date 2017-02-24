#pragma once
#ifndef _COLLECTIONS_RECYCLEARRAY_H
#define _COLLECTIONS_RECYCLEARRAY_H

#include <exception>
#include <stdlib.h>

#ifndef WIN32
#include <memory.h>
#endif

namespace ds{

/* RECYCLE-ARRAY
	* A simple array that doesn't free memory when it shrinks, instead
	* saving the entries to be reused later.
	* I'd really like to just use STL array or something in the platform,
	* but as far as I know, there are never any guarantees as to how
	* other collections will be implemented, and I specifically want
	* something that avoids memory allocation and deallocation.
	******************************************************************/
template <class T>
class RecycleArray
{
	// Exceptions
public:
	class is_empty : public std::exception { };
	class bad_index : public std::exception { };

public:
	// Clients can optionally supply a factory that will initialize
	// each element, although that only applies if T is a pointer.
	RecycleArray(const size_t growBy = 1);
	RecycleArray(const RecycleArray &rhs);
	virtual ~RecycleArray();

	T*							data();
	const T*					data() const;

	int							size() const;
	bool						setSize(const size_t newSize);
	bool						setSize(const size_t newSize, const T& initialize);
	void						clear();

	T&							operator[](const size_t index) const;

	bool						setTo(const RecycleArray<T>&);

	int							find(const T&, const size_t startIndex = 0) const;

	bool						add(const T&);
	// Note:  This can cause a reorder!
	bool						remove(const T&);
	bool						removeAt(const size_t index);

	// Treat like a list, and keep things ordered
	T							popFront();
	T							popAt(const size_t index);

	// Exchange data between the arrays
	void						swap(RecycleArray<T>&);

	RecycleArray<T>&			operator=(const RecycleArray<T>&);
	bool						operator==(const RecycleArray<T>&) const;

	size_t						alloc() const;

private:
	const size_t				mGrowBy;
	T*							mD;
	size_t						mSize, mAlloc;
};

/* RECYCLE-ARRAY IMPLEMENTATION
	******************************************************************/
template <class T>
RecycleArray<T>::RecycleArray(const size_t growBy)
	: mGrowBy(growBy)
	, mD(nullptr)
	, mSize(0)
	, mAlloc(0)
{
}

template <typename T>
RecycleArray<T>::RecycleArray(const RecycleArray &rhs)
	: mGrowBy(rhs.mGrowBy)
	, mD(nullptr)
	, mSize(0)
	, mAlloc(0)
{
	setTo(rhs);
}

template <class T>
RecycleArray<T>::~RecycleArray()
{
	if(mD) realloc(mD, 0);
}

template <class T>
inline T* RecycleArray<T>::data()
{
	return mD;
}

template <class T>
inline const T* RecycleArray<T>::data() const
{
	return mD;
}

template <class T>
inline int RecycleArray<T>::size() const
{
	return mSize;
}

template <class T>
bool RecycleArray<T>::setSize(const size_t newSize)
{
	if(newSize <= mAlloc) {
		mSize = newSize;
		return true;
	}
	const size_t	newAlloc = newSize + mGrowBy;
	T*				newD = (T*)realloc(mD, sizeof(T)*newAlloc);
	if(newD == NULL) return false;

	mD = newD;
	mSize = newSize;
	mAlloc = newAlloc;
	return true;
}

template <class T>
bool RecycleArray<T>::setSize(const size_t newSize, const T& initialize)
{
	const int			oldSize = mSize;
	if(!setSize(newSize)) return false;
	for(int k = oldSize; k < mSize; k++) mD[k] = initialize;
	return true;
}

template <class T>
T& RecycleArray<T>::operator[](const size_t index) const
{
	if(index < 0 || index >= mSize) throw bad_index();
	return mD[index];
}

template <class T>
void RecycleArray<T>::clear()
{
	mSize = 0;
}

template <class T>
bool RecycleArray<T>::setTo(const RecycleArray<T>& src)
{
	if(this == &src) return true;
	if(!setSize(src.mSize)) return false;
	if(mSize > 0) memcpy(mD, src.mD, sizeof(T)*mSize);
	return true;
}

template <class T>
int RecycleArray<T>::find(const T& t, const size_t startIndex) const
{
	for(int k = startIndex; k < mSize; k++) {
		if(mD[k] == t) return k;
	}
	return -1;
}

template <class T>
bool RecycleArray<T>::add(const T& t)
{
	if(mSize < mAlloc) {
		mD[mSize++] = t;
		return true;
	}
	const size_t oldSize = mSize;
	if(!setSize(mSize + 1)) return false;
	mD[oldSize] = t;
	return true;
}

template <class T>
bool RecycleArray<T>::remove(const T& t)
{
	if(mSize < 1) return false;
	// Pop from last is easiest
	if(mD[mSize - 1] == t) {
		mSize--;
		return true;
	}
	// Otherwise find and replace with last
	for(int k = 0; k < mSize - 1; k++) {
		if(mD[k] == t) {
			mD[k] = mD[mSize - 1];
			mSize--;
			return true;
		}
	}
	return false;
}

template <class T>
bool RecycleArray<T>::removeAt(const size_t idx)
{
	if(mSize < 1 || idx < 0 || idx >= mSize) return false;
	mD[idx] = mD[mSize - 1];
	mSize--;
	return true;
}

template <class T>
T RecycleArray<T>::popFront()
{
	return popAt(0);
}

template <class T>
T RecycleArray<T>::popAt(const size_t index)
{
	if(index < 0 || index >= mSize) throw bad_index();

	T			ans = mD[index];
	for(int k = index + 1; k < mSize; k++) mD[k - 1] = mD[k];
	mSize--;
	return ans;
}

template <class T>
void RecycleArray<T>::swap(RecycleArray<T>& o)
{
	T*				d = mD;			mD = o.mD;			o.mD = d;
	const size_t	s = mSize;		mSize = o.mSize;	o.mSize = s;
	const size_t	a = mAlloc;		mAlloc = o.mAlloc;	o.mAlloc = a;
}

template <class T>
RecycleArray<T>& RecycleArray<T>::operator=(const RecycleArray<T>& o)
{
	setTo(o);
	return *this;
}

template <class T>
bool RecycleArray<T>::operator==(const RecycleArray<T>& o) const
{
	if(mSize != o.mSize) return false;
	if(mSize < 1) return true;
	return memcmp(mD, o.mD, sizeof(T)*mSize) == 0;
}

template <class T>
inline size_t RecycleArray<T>::alloc() const
{
	return mAlloc;
}

} // namespace ds

#endif // RECYCLEARRAY_H
