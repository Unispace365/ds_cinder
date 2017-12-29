#include "stdafx.h"

#include "ds/data/resource_list.h"

#include "ds/query/query_client.h"
#include "ds/query/query_result.h"

namespace ds {

/**
 * ds::ResourceList
 */
ResourceList::ResourceList()
{
}

void ResourceList::clear()
{
  mData.clear();
}

bool ResourceList::get(const Resource::Id& id, Resource& ans)
{
  // For some reason unordered_map throws an exception when trying to find() on an empty map.
  // I think it's probably a library error.
  if (!mData.empty()) {
    auto it = mData.find(id);
    if (it != mData.end()) {
      ans = it->second;
      return true;
    }
  }
  return query(id, ans);
}

bool ResourceList::query(const Resource::Id& id, Resource& ans)
{
  // XXX Replace with ans.query() when I get a chance to verify that it works fine

  const std::string&          dbPath = id.getDatabasePath();
  if (dbPath.empty()) return false;

  mBuf.str("");
  mBuf << "SELECT resourcestype,resourcesduration,resourceswidth,resourcesheight,resourcesfilename,resourcespath FROM Resources WHERE resourcesid = " << id.mValue;
  query::Result               r;
  if (!query::Client::query(dbPath, mBuf.str(), r) || r.rowsAreEmpty()) return false;

  query::Result::RowIterator  it(r);
  if (!it.hasValue()) return false;

  ans.setDbId(id);
  ans.setTypeFromString(it.getString(0));
  ans.mDuration = it.getFloat(1);
  ans.mWidth = it.getFloat(2);
  ans.mHeight = it.getFloat(3);
  ans.mFileName = it.getString(4);
  ans.mPath = it.getString(5);

  try {
    mData[id] = ans;
  } catch (std::exception const&) {
  }

  return true;
}

} // namespace ds
