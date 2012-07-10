#pragma once
#ifndef _COLLECTIONS_MANAGEDFACTORY_H
#define _COLLECTIONS_MANAGEDFACTORY_H

#pragma once

/* MANAGED-FACTORY
 * The basic memory management behaviour for a managed list.
 * implementing newObject() and deleteObject() is essential,
 * and most clients will want setObject(), but the remaining
 * functions depend on the subclass and use.
 ******************************************************************/
template <class T>
class ManagedFactory
{
public:
	// If sort is true, then clients must implement sort()
	explicit ManagedFactory(const bool sort = false) : mSort(sort) { }
	virtual ~ManagedFactory() { }

	const bool		mSort;

	virtual T		nullObject() const = 0;
	virtual T		newObject() const = 0;
	virtual void	deleteObject(T) const = 0;
	// Set the object to its initial, empty state
	virtual void	initializeObject(T) const = 0;
	// Set dst to const src.  Note that src can be NULL.
	virtual void	setObject(T dst, T src) const = 0;
	// Only necessary if mSort is set to true. -1 for <, 0 for =, 1 for >
	virtual int		sortObject(T, T) const { return -1; }
};

#endif // MANAGEDFACTORY_H
