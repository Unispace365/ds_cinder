#pragma once
#ifndef DS_UI_FBO_H
#define DS_UI_FBO_H
#include "cinder/gl/Texture.h"

namespace ds {
namespace ui {

class FboGeneral
{
public:
	FboGeneral();
	~FboGeneral();

	void             setup(bool useDepth = false, bool useStencil = false);

	void             attach(ci::gl::TextureRef target, bool useDepth = false, bool useStencil = false);
	void             detach();

	void             begin();
	void             end();

	ci::gl::TextureRef getAttached();

	void             offsetViewport(int offsetX, int offsetY);

	void             pushTransformation();
	void             popTransformation();

	int              getWidth() const;
	int              getHeight() const;
private:
	FboGeneral(const FboGeneral &rhs){}
	FboGeneral &operator =(const FboGeneral &rhs){}

	void             activate();
	void             deactivate();

	void             reset();

	int              mWidth;
	int              mHeight;

	GLuint           mFboId;
	GLuint           mDepthId;
	GLuint           mStencilId;
	GLint            mPreviousFbo;
	ci::gl::TextureRef mAttached;
public:
	// Avoid making clients remember to call required functions
	class AutoAttach
	{
	public:
		AutoAttach(FboGeneral &, ci::gl::TextureRef target, bool useDepth = false, bool useStencil = false);
		~AutoAttach();
	private:
		FboGeneral &mFbo;
	};

	class AutoRun
	{
	public:
		AutoRun(FboGeneral &);
		~AutoRun();
	private:
		FboGeneral &mFbo;
	};
};

}
}

#endif//DS_UI_FBO_H
