#include "stdafx.h"

#include "ds/data/resource_list.h"

#include "ds/query/query_client.h"
#include "ds/query/query_result.h"

namespace ds {

ResourceList::ResourceList()
{
}

void ResourceList::clear(){
  mData.clear();
}

bool ResourceList::get(const Resource::Id& id, Resource& ans){
	if(!mData.empty()) {
		auto it = mData.find(id);
		if(it != mData.end()) {
			ans = it->second;
			return true;
		}
	}
	return query(id, ans);
}

bool ResourceList::query(const Resource::Id& id, Resource& ans){
	const std::string&          dbPath = id.getDatabasePath();
	if(dbPath.empty()) return false;
	std::stringstream buf;
	buf.str("");
	buf << "SELECT resourcestype,resourcesduration,resourceswidth,resourcesheight,resourcesfilename,resourcespath FROM Resources WHERE resourcesid = " << id.mValue;
	query::Result               r;
	if(!query::Client::query(dbPath, buf.str(), r) || r.rowsAreEmpty()) return false;

	query::Result::RowIterator  it(r);
	if(!it.hasValue()) return false;

	ans.setDbId(id);
	ans.setTypeFromString(it.getString(0));
	ans.mDuration = it.getFloat(1);
	ans.mWidth = it.getFloat(2);
	ans.mHeight = it.getFloat(3);
	ans.mFileName = it.getString(4);
	ans.mPath = it.getString(5);

	mData[id] = ans;

	return true;
}

} // namespace ds
