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

private:
	typedef ds::App		inherited;
	ds::ui::Pdf*		mPdf;
};

BasicTweenApp::BasicTweenApp()
		: mPdf(nullptr) {
}

void BasicTweenApp::setupServer() {
	std::cout << "Press \"+\" and \"-\" to navigate pages." << std::endl;
	std::cout << "Hmm for some reason it's not getting key presses. Use mouse left/right instead." << std::endl;

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();

	ds::ui::Pdf&	pdf = ds::ui::Sprite::makeAlloc<ds::ui::Pdf>([this]()->ds::ui::Pdf*{return new ds::ui::Pdf(this->mEngine);}, &rootSprite);
	mPdf = &pdf;
#if 0
	// By default the PDF sprite will use the first page size, and scale any subsequent...
	pdf.setResourceFilename(ds::Environment::getAppFolder("data", "bitcoin.pdf"));
#else
	// Or you can turn on auto resize and the sprite will resize when the page size changes
	pdf.setPageSizeMode(ds::ui::Pdf::kAutoResize);
	pdf.setResourceFilename(ds::Environment::getAppFolder("data", "multi_sizes.pdf"));
	pdf.setPageSizeChangedFn([](){std::cout << "change" << std::endl;});
#endif
	pdf.setCenter(0.5f, 0.5f);
	pdf.setPosition(floorf(mEngine.getWorldWidth()/2.0f), floorf(mEngine.getWorldHeight()/2.0f));
}

void BasicTweenApp::keyDown(KeyEvent e) {
	inherited::keyDown(e);

	if (e.getCode() == KeyEvent::KEY_ESCAPE) {
		quit();
	} else if (e.getCode() == KeyEvent::KEY_PLUS || e.getCode() == KeyEvent::KEY_n || e.getCode() == KeyEvent::KEY_RIGHT) {
		if (mPdf) mPdf->goToNextPage();
	} else if (e.getCode() == KeyEvent::KEY_MINUS || e.getCode() == KeyEvent::KEY_p || e.getCode() == KeyEvent::KEY_LEFT) {
		if (mPdf) mPdf->goToPreviousPage();
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
