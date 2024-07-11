#pragma once

#include <Poco/Runnable.h>
#include <ds/query/query_result.h>
#include <functional>

namespace waffles {

/**
 * \class waffles::FileListSaveService
 *			Load all the files and folders in a directory
 */
class FileListSaveService : public Poco::Runnable {
  public:
	FileListSaveService();

	virtual void run();
	void		 setFileList(const ci::XmlTree& fileList);

  private:
	ci::XmlTree mFileList;
};

} // namespace waffles
