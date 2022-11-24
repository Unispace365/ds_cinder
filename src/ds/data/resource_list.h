#pragma once
#ifndef DS_DATA_RESOURCELIST_H_
#define DS_DATA_RESOURCELIST_H_

#include "ds/data/resource.h"
#include <sstream>
#include <unordered_map>

namespace ds {

/**
 * \class ResourceList
 * \brief A caching collection of resources.
 */
class ResourceList {
  public:
	ResourceList();

	void clear();
	bool get(const Resource::Id&, Resource&);

  private:
	std::unordered_map<Resource::Id, ds::Resource> mData;

	bool query(const Resource::Id&, Resource&);
};

} // namespace ds

#endif // DS_DATA_RESOURCELIST_H_
