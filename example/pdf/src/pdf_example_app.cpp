#include <ds/app/app.h>
#include <ds/app/engine/engine.h>
#include <ds/app/environment.h>
#include <ds/ui/sprite/pdf.h>

using namespace std;
using namespace ci;
using namespace ci::app;

class BasicTweenApp : public ds::App {
public:
	BasicTweenApp();

	void				setupServer();
	void                keyDown(KeyEvent);
	virtual void		mouseUp(MouseEvent);
	virtual void		fileDrop(ci::app::FileDropEvent event);

private:
	typedef ds::App		inherited;
	ds::ui::Pdf*		mPdf;
};

BasicTweenApp::BasicTweenApp()
		: mPdf(nullptr) {
}

void BasicTweenApp::setupServer() {
	std::cout << "Press \"+\" and \"-\" to navigate pages." << std::endl;

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();

	ds::ui::Pdf&	pdf = *(new ds::ui::Pdf(this->mEngine));
	rootSprite.addChild(pdf);
	mPdf = &pdf;
#if 0
	// By default the PDF sprite will use the first page size, and scale any subsequent...
	//pdf.setResourceFilename(ds::Environment::getAppFolder("data", "bitcoin.pdf"));
#else
	// Or you can turn on auto resize and the sprite will resize when the page size changes
//	pdf.setPageSizeMode(ds::ui::Pdf::kAutoResize);
	pdf.setResourceFilename(ds::Environment::getAppFolder("data", "multi_sizes.pdf"));
	pdf.setPageSizeChangedFn([](){std::cout << "Page size changed" << std::endl;});
#endif
	pdf.setCenter(0.5f, 0.5f);
	pdf.setPosition(floorf(mEngine.getWorldWidth()/2.0f), floorf(mEngine.getWorldHeight()/2.0f));
}

void BasicTweenApp::fileDrop(ci::app::FileDropEvent event){
	for(auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it){
		if(mPdf){
			mPdf->setResourceFilename((*it).string());
			break; //only use the first file
		}
	}
}

void BasicTweenApp::keyDown(KeyEvent e) {
	inherited::keyDown(e);

	if (e.getCode() == KeyEvent::KEY_ESCAPE) {
		quit();
	} else if (e.getCode() == KeyEvent::KEY_PLUS || e.getCode() == KeyEvent::KEY_n || e.getCode() == KeyEvent::KEY_RIGHT) {
		if (mPdf) mPdf->goToNextPage();
	} else if (e.getCode() == KeyEvent::KEY_MINUS || e.getCode() == KeyEvent::KEY_p || e.getCode() == KeyEvent::KEY_LEFT) {
		if (mPdf) mPdf->goToPreviousPage();
	} else if(e.getCode() == KeyEvent::KEY_l){
		if(mPdf) mPdf->setResourceFilename(ds::Environment::expand("%APP%/data/bitcoin.pdf"));
	}
}

void BasicTweenApp::mouseUp(MouseEvent e) {
	inherited::mouseUp(e);

	if (mPdf) {
		if (e.isLeft()) mPdf->goToPreviousPage();
		else if (e.isRight()) mPdf->goToNextPage();
	}
}

// This line tells Cinder to actually create the application
CINDER_APP_BASIC( BasicTweenApp, RendererGl )
