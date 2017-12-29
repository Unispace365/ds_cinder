#pragma once
#ifndef DS_UI_SPRITE_NINEPATCH_H_
#define DS_UI_SPRITE_NINEPATCH_H_

#include <string>
#include "sprite.h"
#include "ds/ui/image_source/image_owner.h"

namespace ds {
namespace ui {

/**
 * \class ds::ui::NinePatch
 * \brief Section an image into 9 parts and draw them, generally
 * stretching the middle to accomodate arbitrary sizes. Used for
 * drawing buttons (i.e. rounded corners).
 */
class NinePatch : public Sprite
				, public ImageOwner {
public:
	static NinePatch&					makeNinePatch(SpriteEngine&, Sprite *parent = nullptr);
	static NinePatch&					makeNinePatch(SpriteEngine&, const std::string &file, Sprite *parent = nullptr);
	NinePatch(SpriteEngine&);

	virtual void						onUpdateServer(const UpdateParams&) override;
	virtual void						drawLocalClient();

	struct Status {
		static const int				STATUS_EMPTY = 0;
		static const int				STATUS_LOADED = 1;
		int								mCode;
	};
	void								setStatusCallback(const std::function<void(const Status&)>&);

protected:
	virtual void						onSizeChanged();
	virtual void						onImageChanged();
	virtual void						writeAttributesTo(ds::DataBuffer&);
	virtual void						readAttributeFrom(const char attributeId, ds::DataBuffer&);

private:
	void								setStatus(const int);
	void								init();

	Status								mStatus;
	bool								mStatusDirty;
	std::function<void(const Status&)>	mStatusFn;

	bool								mSizeDirty;

	/**
	 * \class ds::ui::NinePatch::Cell
	 * Describe a single set of drawing instructions for the image.
	 */
	class Cell {
	public:
		Cell();

		ci::vec2						size() const;

		void							draw(const ci::gl::TextureRef);
		void							print(const int tabs) const;

		bool							mIsValid;
		ci::Area						mSrc;
		ci::Rectf						mDst;
	};

	/**
	 * \class ds::ui::NinePatch::Patch
	 * A complete darwing description.
	 */
	class Patch {
	public:
		Patch();

		void							clear();
		bool							empty() const;
		void							buildSources(ci::gl::TextureRef);
		void							buildDestinations(const float width, const float height);

		void							draw(const ci::gl::TextureRef);

		void							print(const int tabs = 0) const;

	private:
		static const int				CELL_SIZE = 9;
		Cell							mCell[CELL_SIZE];
		bool							mEmpty;
	};

	Patch								mPatch;

// Engine initialization
public:
	static void							installAsServer(ds::BlobRegistry&);
	static void							installAsClient(ds::BlobRegistry&);
};

} // namespace ui
} // namespace ds

#endif // DS_UI_SPRITE_NINEPATCH_H_
