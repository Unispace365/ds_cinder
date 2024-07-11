#pragma once

#include "file_save_request.h"
#include "file_upload_request.h"
#include <ds/app/event_client.h>
#include <ds/thread/serial_runnable.h>
#include <ds/ui/sprite/sprite_engine.h>

namespace waffles {

/**
 * \class waffles::DrawingUploadService
 *			 Save a drawing to the HD, then upload it to a CMS, if you want
 */
class DrawingUploadService {
  public:
	DrawingUploadService(ds::ui::SpriteEngine&);

  private:
	ds::ui::SpriteEngine&				  mEngine;
	ds::EventClient						  mEventClient;
	ds::SerialRunnable<FileUploadRequest> mUploadRequests;
	ds::SerialRunnable<FileSaveRequest>	  mSaveRequests;
};

} // namespace waffles
