#pragma once
#ifndef PRIVATE_PDFSERVICE_H_
#define PRIVATE_PDFSERVICE_H_

#include <ds/app/engine/engine_service.h>
#include <ds/thread/gl_thread.h>

namespace ds {

class Engine;

namespace pdf {

/**
 * \class ds::pdf::PdfService
 * \brief The engine service object that provides access to the
 * PDF rendering thread.
 */
class Service : public ds::EngineService {
public:
	Service(ds::Engine& engine);
	~Service();

	virtual void		start();

	ds::Engine&			mEngine;

	GlThread			mThread;
};

} // namespace ui
} // namespace ds

#endif // DS_UI_SPRITE_PDF_H_