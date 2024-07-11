#pragma once

#include <Poco/Runnable.h>
#include <ds/query/query_result.h>
#include <functional>

namespace waffles {

/**
 * \class waffles::DirectoryQuery
 *			Load all the files and folders in a directory
 */
class DirectoryQuery : public Poco::Runnable {
  public:
	DirectoryQuery();

	virtual void run();

	void setInputType(const bool getDrives);
	void setDirectoryToLoad(const std::string& directoryPath, const int requestId);

	ds::model::ContentModelRef mOutput;
	ds::model::ContentModelRef mDrives;
	int						   mRequestId;
	bool					   mError;
	bool					   mListDrives;

  private:
	void		query();
	std::string mDirectoryPath;
};

} // namespace waffles
