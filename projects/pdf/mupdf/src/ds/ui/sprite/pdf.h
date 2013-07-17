#pragma once
#ifndef DS_UI_SPRITE_PDF_H_
#define DS_UI_SPRITE_PDF_H_

#include <ds/ui/sprite/sprite.h>

namespace ds {
namespace pdf {
class PdfRes;
class Service;
}

namespace ui {

/**
 * \class ds::ui::sprite::Pdf
 */
class Pdf : public ds::ui::Sprite {
public:
	Pdf(ds::ui::SpriteEngine&);

	Pdf&						setResourceFilename(const std::string& filename);

	virtual void				updateClient(const UpdateParams&);
	virtual void				updateServer(const UpdateParams&);

	// PDF API
	void						setPageNum(const int pageNum);
	int							getPageCount();
	void						goToNextPage();
	void						goToPreviousPage();

protected:
	virtual void				onScaleChanged();
	virtual void				drawLocalClient();

private:
	typedef ds::ui::Sprite		inherited;

	// It'd be nice just have the PdfRes in a unique_ptr,
	// but it has rules around destruction
	class ResHolder {
	public:
		ResHolder(ds::ui::SpriteEngine&);
		~ResHolder();

		void					clear();
		void					setResourceFilename(const std::string& filename);
		void					update();
		void					drawLocalClient();
		void					setScale(const ci::Vec3f&);
		float					getWidth() const;
		float					getHeight() const;
		float					getTextureWidth() const;
		float					getTextureHeight() const;
		void					setPageNum(const int pageNum);
		int						getPageCount();
		void					goToNextPage();
		void					goToPreviousPage();

	private:
		ds::pdf::Service&		mService;
		ds::pdf::PdfRes*		mRes;
	};
	ResHolder					mHolder;
};

} // namespace ui
} // namespace ds

#endif // DS_UI_SPRITE_PDF_H_