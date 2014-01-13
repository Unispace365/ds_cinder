#pragma once
#ifndef _COLLECTIONS_MANAGEDLIST_H
#define _COLLECTIONS_MANAGEDLIST_H

/* MANAGED LIST
 * A templatized set of classes implementing a singly-linked list.
 * It sucks to have a custom collection class, but it's often
 * useful to have a list that does not release memory (a performance
 * killer in realtime environments), and there are never guarantees
 * about how other libraries have implemented their collections.
 * Plus, it's super nice to have all memory management handled
 * automatically.
 ******************************************************************/

#include <assert.h>
#include <exception>
#include "ManagedFactory.h"
#include "RecycleArray.h"
#include "RecycleNode.h"

#pragma once

template <class T>
class ManagedList;

template <class T>
class ManagedListIterator;

/* MANAGED-CONDITION
 * General accept/reject rule
 ******************************************************************/
template <class T>
class ManagedCondition
{
public:
	virtual ~ManagedCondition() { }
	virtual bool	accept(T&) = 0;
};

/* MANAGED-COMBINER
 * Utility for the removeDuplicates() function.
 ******************************************************************/
template <class T>
class ManagedCombiner
{
public:
	virtual ~ManagedCombiner() { }
	// If the two objects are equal, combine them into the first
	// (the second will be tossed) and return true.
	virtual bool	combine(T, T) = 0;
};

/* MANAGED-ARRAY-OPERATION
 * Present the list as an array to be manipulated.
 ******************************************************************/
template <class T>
class ManagedArrayOperation
{
public:
	virtual ~ManagedArrayOperation() { }
	// If the two objects are equal, combine them into the first
	// (the second will be tossed) and return true.
	virtual void		on(RecycleArray<T>&) = 0;

private:
	friend class ManagedList<T>;
	RecycleArray<T>		mScratch;
};

class ManagedListBase
{
// Exceptions
public:
	class is_empty : public std::exception { };
	class bad_index : public std::exception { };

public:
	// Move function.  Pick a list and a node for both src and dst.
	static const int			ACTIVE_LIST = (1<<0);
	static const int			RETIRED_LIST = (1<<1);
	static const int			ALL_NODE = (1<<10);
	static const int			FRONT_NODE = (1<<11);
	static const int			BACK_NODE = (1<<12);

public:
	virtual ~ManagedListBase()	{ }

protected:
	ManagedListBase()			{ }
};

/* MANAGED-LIST
 * A simple list that doesn't free memory when it shrinks, instead
 * saving the entries to be reused later.  Clients supply an object
 * for allocating and deallocating template memory.
 * NOTE:  This class MUST operate on pointer types.  I should
 * enforce that, but first I want to get things working.
 ******************************************************************/
template <class T>
class ManagedList : public ManagedListBase
{
public:
	// The factory handles all memory management for this class.
	// Note that this class DOES NOT OWN the factory.  The client
	// is responsible for deleting it.  This is because factories
	// have no state, so you can create a static object and reuse.
	ManagedList(const ManagedFactory<T>&);
	virtual ~ManagedList();

	bool						isEmpty() const;
	int							size() const;
	int							retiredSize() const;

	virtual void				clear();

	T							front();
	T							back();
	T							at(const int index);
	const T						at(const int index) const;

	// Increase the size of the list with a new object at the front.
	T							pushFront();
	// Increase the size of the list with a new object at the back.
	T							pushBack();
	// Add after the item at the given index
	T							pushAt(int index);
	// Same as above, but call ManagedFactory::set()
	T							pushBack(T const);
	// This is the required variant if sorting in the factory is true.
	// You need to provide a source to copy from, which is pretty annoying.
	void						pushSorted(T const);

	void						popFront();
	void						popBack();
	void						pop(T);

	// Swap the active data.  If retired is true, swap that, too.
	void						swap(ManagedList<T>&, const bool retired = false);

	// Move from me to the destination list, with control over
	// the list type, which nodes, and where.
	// i.e move(dst, ACTIVE_LIST | ALL_NODE, RETIRED_LIST | FRONT_NODE);
	void						move(ManagedList<T>&, const int from, const int to);
	// Move a specific item from me to dest.  Insert index of < 0 is head,
	// otherwise insert at the given position.
	bool						move(const T&, ManagedList<T>& dst, const int insertIndex = -1);

	// Remove everyone that matches the condition
	void						remove(ManagedCondition<T>&);

	// Provide the list as an array to be manipulated
	void						arrayOperation(ManagedArrayOperation<T>&);

	// Utility for removing duplicates, as designated by the combine() function
	// on the factory.
	void						removeDuplicates(ManagedCombiner<T>&);

	ManagedListIterator<T>		begin() const;

private:
	typedef prvlist::_node<T>	Node;

	Node*						mHead;
	Node*						mRecycle;

	const ManagedFactory<T>&	mFactory;

	// Answer a new node, either popped from the recycle list
	// or allocated.
	Node*						newNode();
	void						deleteAll(Node*);

	// Exchange my head and recycled list; only used internally to
	// make some algorithms less redundant.
	void						exchange();

// internal for the iterator
public:
	const Node*					getHead() const;
};

/* MANAGED-LIST-ITERATOR
 * No, not STL compatible at the moment.
 ******************************************************************/
template <class T>
class ManagedListIterator
{
public:
	ManagedListIterator(const ManagedList<T>&);
	ManagedListIterator(const ManagedListIterator<T>&);

	void						operator++();
	void						operator+=(const int count);
	ManagedListIterator<T>&		operator=(const ManagedList<T>&);
	ManagedListIterator<T>&		operator=(const ManagedListIterator<T>&);

	bool						hasValue() const;
	const T						operator*() const;

	bool						hasNext() const;
	const T						peekNext() const;

private:
	ManagedListIterator();
	typedef prvlist::_node<T>	Node;

	const Node*					mN;
};

/* MANAGED-LIST IMPLEMENTATION
 ******************************************************************/
template <class T>
ManagedList<T>::ManagedList(const ManagedFactory<T>& f)
	: mHead(NULL)
	, mRecycle(NULL)
	, mFactory(f)
{
}

template <class T>
ManagedList<T>::~ManagedList()
{
	deleteAll(mHead);
	deleteAll(mRecycle);
}

template <class T>
bool ManagedList<T>::isEmpty() const
{
	return mHead == NULL;
}

template <class T>
int ManagedList<T>::size() const
{
	int						ans = 0;
	prvlist::_node_base*	n = mHead;
	while (n != NULL) {
		ans++;
		n = n->mNext;
	}
	return ans;
}

template <class T>
int ManagedList<T>::retiredSize() const
{
	int						ans = 0;
	prvlist::_node_base*	n = mRecycle;
	while (n != NULL) {
		ans++;
		n = n->mNext;
	}
	return ans;
}

template <class T>
void ManagedList<T>::clear()
{
	if (mHead == NULL) return;
	if (mRecycle == NULL) mRecycle = mHead;
	else mRecycle->tail()->mNext = mHead;
	mHead = NULL;
}

template <class T>
T ManagedList<T>::front()
{
	if (mHead == NULL) throw is_empty();
	return mHead->mData;
}

template <class T>
T ManagedList<T>::back()
{
	if (mHead == NULL) throw is_empty();
	return ((Node*)mHead->tail())->mData;
}

template <class T>
T ManagedList<T>::at(const int index)
{
	int			idx = index;
	Node*		n = mHead;
	while (n != NULL) {
		if (idx == 0) return n->mData;
		idx--;
		n = (Node*)(n->mNext);
	}
	throw bad_index();
}

template <class T>
const T ManagedList<T>::at(const int index) const
{
	int				idx = index;
	const Node*		n = mHead;
	while (n != NULL) {
		if (idx == 0) return n->mData;
		idx--;
		n = (const Node*)(n->mNext);
	}
	throw bad_index();
}

template <class T>
T ManagedList<T>::pushFront()
{
	assert(!mFactory.mSort);

	Node*			n = newNode();
	n->mNext = mHead;
	mHead = n;

	mFactory.initializeObject(n->mData);
	return n->mData;
}

template <class T>
T ManagedList<T>::pushBack()
{
	assert(!mFactory.mSort);

	Node*			n = newNode();
	if (mHead == NULL) mHead = n;
	else mHead->tail()->mNext = n;

	mFactory.initializeObject(n->mData);
	return n->mData;
}

template <class T>
T ManagedList<T>::pushAt(int index)
{
	assert(!mFactory.mSort);
	if (isEmpty() || index < 0) {
		return pushFront();
	}

	Node*				prv = mHead;
	while (prv != NULL) {
		Node*			nxt = (Node*)(prv->mNext);
		if (index-- <= 0) {
			Node*		n = newNode();
			n->mNext = prv->mNext;
			prv->mNext = n;

			mFactory.initializeObject(n->mData);
			return n->mData;
		}
		prv = nxt;
	}
	return pushBack();
}

template <class T>
T ManagedList<T>::pushBack(T const src)
{
	T				ans = pushBack();
	mFactory.setObject(ans, src);
	return ans;
}

template <class T>
void ManagedList<T>::pushSorted(T const src)
{
	assert(mFactory.mSort);

	Node*			n = newNode();
	mFactory.setObject(n->mData, src);

	if (mHead == NULL || mFactory.sortObject(n->mData, mHead->mData) < 0) {
		n->mNext = mHead;
		mHead = n;
	} else {
		Node*		prv = mHead;
		while (prv->mNext != NULL) {
			Node*	nxt = (Node*)(prv->mNext);
			if (mFactory.sortObject(n->mData, nxt->mData) < 0) {
				n->mNext = nxt;
				prv->mNext = n;
				return;
			}
			prv = nxt;
		}
		prv->mNext = n;
	}
}

template <class T>
void ManagedList<T>::popFront()
{
	if (mHead == NULL) throw is_empty();
	Node*		ans = mHead;
	mHead = (Node*)(mHead->mNext);
	ans->mNext = mRecycle;
	mRecycle = ans;
}

template <class T>
void ManagedList<T>::popBack()
{
	if (mHead == NULL) throw is_empty();
	pop(back());
}

template <class T>
void ManagedList<T>::pop(T src)
{
	if (mHead == NULL) throw is_empty();

	Node*			popped = NULL;
	if (src == mHead->mData) {
		popped = mHead;
		mHead = (Node*)(popped->mNext);
		popped->mNext = NULL;
	} else {
		Node*		n = mHead;
		while (n != NULL && n->mNext != NULL) {
			Node*	nxt = (Node*)(n->mNext);
			if (src == nxt->mData) {
				popped = nxt;
				n->mNext = (Node*)(popped->mNext);
				popped->mNext = NULL;
				break;
			}
			n = nxt;
		}
	}
	if (popped != NULL) {
		popped->mNext = mRecycle;
		mRecycle = popped;
	} else throw bad_index();
}

template <class T>
void ManagedList<T>::swap(ManagedList<T>& o, const bool retired)
{
	if (this != &o) {
		Node*		n = mHead; mHead = o.mHead; o.mHead = n;
		if (retired) {
					n = mRecycle; mRecycle = o.mRecycle; o.mRecycle = n;
		}
	}
}

template <class T>
void ManagedList<T>::move(ManagedList<T>& dst, const int from, const int to)
{
	if (this == &dst) return;

	// Always operate on the active list, so if the retired is requested, switch.
	if ((from&RETIRED_LIST) != 0) exchange();
	if ((to&RETIRED_LIST) != 0) dst.exchange();

	Node*				n = NULL;
	if ((from&ALL_NODE) != 0) {
		n = mHead;
		mHead = NULL;
	} else if ((from&FRONT_NODE) != 0) {
		if (mHead) {
			n = mHead;
			mHead = (Node*)(mHead->mNext);
			n->mNext = NULL;
		}
	} else if ((from&BACK_NODE) != 0) {
		// Not currently supported
	}

	if (n != NULL) {
		if ((to&FRONT_NODE) != 0) {
			n->tail()->mNext = dst.mHead;
			dst.mHead = n;
		// Default to adding to the back
		} else {
			if (dst.mHead == NULL) dst.mHead = n;
			else dst.mHead->tail()->mNext = n;
		}
	}

	// Restore proper
	if ((from&RETIRED_LIST) != 0) exchange();
	if ((to&RETIRED_LIST) != 0) dst.exchange();
}

template <class T>
bool ManagedList<T>::move(const T& d, ManagedList<T>& dst, const int insertIndex)
{
	if (mHead == NULL) return false;

	// Extract the desired node from the source list
	Node*				popped = NULL;
	if (mHead->mData == d) {
		popped = mHead;
		mHead = (Node*)(mHead->mNext);
	} else {
		Node*			n = mHead;
		while (n != NULL && n->mNext != NULL) {
			Node*		nxt = (Node*)(n->mNext);
			if (d == nxt->mData) {
				popped = nxt;
				n->mNext = (Node*)(popped->mNext);
				break;
			}
			n = nxt;
		}
	}

	if (!popped) return false;
	popped->mNext = NULL;

	// Add it at the desired location in the dest list
	if (insertIndex < 0 || dst.mHead == NULL) {
		popped->mNext = dst.mHead;
		dst.mHead = popped;
	} else {
		int			idx = insertIndex;
		Node*		prv = dst.mHead;
		while (prv && prv->mNext) {
			if (idx-- <= 0) {
				popped->mNext = prv->mNext;
				prv->mNext = popped;
				return true;
			}
			prv = (Node*)(prv->mNext);
		}
		// fallback -- add to tail
		prv->mNext = popped;
	}

	return true;
}

template <class T>
void ManagedList<T>::remove(ManagedCondition<T>& cnd)
{
	// Pop off all the heads
	if (!mHead) return;
	while (mHead && cnd.accept(mHead->mData)) {
		Node*		pop = mHead;
		mHead = (Node*)(mHead->mNext);
		pop->mNext = mRecycle;
		mRecycle = pop;
	}

	// Pop all interiors
	Node*			prv = mHead;
	while (prv != NULL && prv->mNext != NULL) {
		if (cnd.accept(((Node*)prv->mNext)->mData)) {
			Node*	pop = (Node*)(prv->mNext);
			prv->mNext = pop->mNext;
			pop->mNext = mRecycle;
			mRecycle = pop;
		} else {
			prv = (Node*)(prv->mNext);
		}
	}
}

template <class T>
void ManagedList<T>::arrayOperation(ManagedArrayOperation<T>& op)
{
	const int			listSize = size();
	if (listSize < 1 || !op.mScratch.setSize(listSize)) return;
	
	T*					d = op.mScratch.data();
	Node*				n = mHead;
	int					idx = 0;
	// Place everyone in the array
	while (n) {
		d[idx++] = n->mData;
		n = (Node*)(n->mNext);
	}

	// Run the operation
	op.on(op.mScratch);

	// Take everyone from the array and place them back in the list
	// at the new positions.
	n = mHead;
	idx = 0;
	while (n) {
		n->mData = d[idx++];
		n = (Node*)(n->mNext);
	}
}

template <class T>
void ManagedList<T>::removeDuplicates(ManagedCombiner<T>& cmb)
{
	// We always retain the previous.
	Node*			prv = mHead;
	while (prv != NULL && prv->mNext != NULL) {
		Node*		pop = (Node*)(prv->mNext);
		while (pop != NULL && cmb.combine(prv->mData, pop->mData)) {
			prv->mNext = pop->mNext;
			pop->mNext = mRecycle;
			mRecycle = pop;
			pop = (Node*)(prv->mNext);
		}

		prv = (Node*)(prv->mNext);
	}
}

template <class T>
ManagedListIterator<T> ManagedList<T>::begin() const
{
	return ManagedListIterator<T>(*this);
}

template <class T>
prvlist::_node<T>* ManagedList<T>::newNode()
{
	Node*			n = NULL;
	if (mRecycle != NULL) {
		n = mRecycle;
		mRecycle = (Node*)(mRecycle->mNext);
		n->mNext = NULL;
	} else {
		n = new Node();
		if (n == NULL) throw is_empty();
		n->mData = mFactory.newObject();
	}
	return n;
}

template <class T>
void ManagedList<T>::deleteAll(Node* n)
{
	while (n != NULL) {
		Node*		next = (Node*)(n->mNext);
		n->mNext = NULL;
		mFactory.deleteObject(n->mData);
		delete n;
		n = next;
	}
}

template <class T>
void ManagedList<T>::exchange()
{
	Node*		n = mHead;		mHead = mRecycle;	mRecycle = n;
}

template <class T>
const prvlist::_node<T>* ManagedList<T>::getHead() const
{
	return mHead;
}

/* MANAGED-LIST-ITERATOR IMPLEMENTATION
 ******************************************************************/
template <class T>
ManagedListIterator<T>::ManagedListIterator(const ManagedList<T>& l)
	: mN(l.getHead())
{
}

template <class T>
ManagedListIterator<T>::ManagedListIterator(const ManagedListIterator<T>& o)
	: mN(o.mN)
{
}

template <class T>
ManagedListIterator<T>& ManagedListIterator<T>::operator=(const ManagedList<T>& l)
{
	mN = l.getHead();
	return *this;
}

template <class T>
ManagedListIterator<T>& ManagedListIterator<T>::operator=(const ManagedListIterator<T>& o)
{
	if (this != &o) mN = o.mN;
	return *this;
}

template <class T>
void ManagedListIterator<T>::operator++()
{
	if (mN != nullptr) mN = (Node*)(mN->mNext);
}

template <class T>
void ManagedListIterator<T>::operator+=(const int count) {
	for (int k=0; k<count; ++k) {
		if (mN == nullptr) return;
		mN = (Node*)(mN->mNext);
	}
}

template <class T>
bool ManagedListIterator<T>::hasValue() const
{
	return mN != NULL;
}

template <class T>
const T ManagedListIterator<T>::operator*() const
{
	if (mN == NULL) return NULL;
	return mN->mData;
}

template <class T>
bool ManagedListIterator<T>::hasNext() const
{
	return mN != NULL && mN->mNext != NULL;
}

template <class T>
const T ManagedListIterator<T>::peekNext() const
{
	if (!hasNext()) return NULL;
	const Node*		nxt = (Node*)(mN->mNext);
	return nxt->mData;
}

#endif // MANAGEDLIST_H
