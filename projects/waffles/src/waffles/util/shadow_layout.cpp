#include "stdafx.h"

#include "shadow_layout.h"

#include <cinder/CinderImGui.h>

#include <ds/app/environment.h>
#include <ds/data/color_list.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/util/clip_plane.h>
#include <ds/util/color_util.h>
#include <ds/util/string_util.h>


namespace {
auto INIT_RIGHT = []() {
	DS_LOG_INFO("Adding shadow layout sprite");
	ds::App::AddStartup([](ds::Engine& e) {
		e.installSprite([](ds::BlobRegistry& r) { waffles::ShadowLayout::installAsServer(r); },
						[](ds::BlobRegistry& r) { waffles::ShadowLayout::installAsClient(r); });

		e.registerSpriteImporter("shadow_layout", [](ds::ui::SpriteEngine& enginey) -> ds::ui::Sprite* {
			if (enginey.getAppSettings().getBool("shadows:enabled", 0, true)) {
				return new waffles::ShadowLayout(enginey);
			} else {
				return new ds::ui::LayoutSprite(enginey);
			}
		});

		e.registerSpritePropertySetter<waffles::ShadowLayout>(
			"shadow_size", [](waffles::ShadowLayout& texty, const std::string& theValue,
							  const std::string& fileReferrer) { texty.setShadowSize(ds::string_to_int(theValue)); });

		e.registerSpritePropertySetter<waffles::ShadowLayout>(
			"shadow_iterations",
			[](waffles::ShadowLayout& texty, const std::string& theValue, const std::string& fileReferrer) {
				texty.setShadowIterations(ds::string_to_int(theValue));
			});

		e.registerSpritePropertySetter<waffles::ShadowLayout>(
			"shadow_opacity",
			[](waffles::ShadowLayout& texty, const std::string& theValue, const std::string& fileReferrer) {
				texty.setShadowOpacity(ds::string_to_float(theValue));
			});

		e.registerSpritePropertySetter<waffles::ShadowLayout>(
			"shadow_sigma",
			[](waffles::ShadowLayout& texty, const std::string& theValue, const std::string& fileReferrer) {
				texty.setShadowSigma(ds::string_to_float(theValue));
			});

		e.registerSpritePropertySetter<waffles::ShadowLayout>(
			"shadow_color",
			[](waffles::ShadowLayout& texty, const std::string& theValue, const std::string& fileReferrer) {
				texty.setShadowColor(ds::parseColor(theValue, texty.getEngine()));
			});

		e.registerSpritePropertySetter<waffles::ShadowLayout>(
			"shadow_offset",
			[](waffles::ShadowLayout& texty, const std::string& theValue, const std::string& fileReferrer) {
				texty.setShadowOffset(ci::vec2(ds::parseVector(theValue)));
			});

		e.registerSpritePropertySetter<waffles::ShadowLayout>(
			"shadow_every_frame",
			[](waffles::ShadowLayout& texty, const std::string& theValue, const std::string& fileReferrer) {
				texty.setShadowEveryFrame(ds::parseBoolean(theValue));
			});
	});
	return true;
}();
}

namespace sub_waffles {
/// these are the same shaders as the text sprite! wee!
const std::string opacityFrag = R"FRAG(
uniform sampler2D	tex0;
in vec4			Color;
in vec2			TexCoord0;
out vec4	    	oColor;
void main()
{
    //oColor = vec4(1.0, 1.0, 1.0, 1.0);
    oColor = texture2D(tex0, vec2(TexCoord0.x, TexCoord0.y) );
    oColor.r *= Color.r;
    oColor.g *= Color.g;
    oColor.b *= Color.b;
    oColor.a *= Color.a;
}
)FRAG";

const std::string vertShader = R"VERT(
uniform mat4		ciModelMatrix;
uniform mat4		ciModelViewProjection;
uniform vec4		uClipPlane0;
uniform vec4		uClipPlane1;
uniform vec4		uClipPlane2;
uniform vec4		uClipPlane3;
in vec4			ciPosition;
in vec4			ciColor;
in vec2			ciTexCoord0;
out vec2			TexCoord0;
out vec4		    Color;
void main()
{
	gl_Position = ciModelViewProjection * ciPosition;
	TexCoord0 = ciTexCoord0;
	Color = ciColor;
	gl_ClipDistance[0] = dot(ciModelMatrix * ciPosition, uClipPlane0);
	gl_ClipDistance[1] = dot(ciModelMatrix * ciPosition, uClipPlane1);
	gl_ClipDistance[2] = dot(ciModelMatrix * ciPosition, uClipPlane2);
	gl_ClipDistance[3] = dot(ciModelMatrix * ciPosition, uClipPlane3);
}
)VERT";

const std::string shadowFrag = R"FRAG(
// Fragment shader
#version 150

uniform sampler2D tex0;
uniform int blurSize;
uniform int horizontalPass;

 // The sigma value for the gaussian function: higher value means more blur
// A good value for 9x9 is around 3 to 5
// A good value for 7x7 is around 2.5 to 4
// A good value for 5x5 is around 2 to 3.5
uniform float sigma;

// The inverse of the texture dimensions along X and Y
uniform vec2 texOffset;

in vec4 Color;
in vec2 TexCoord0;
out vec4 oColor;

const float pi = 3.14159265;

void main() {

    vec4 vertTexCoord = vec4(TexCoord0.x, TexCoord0.y, 0, 0);
    float numBlurPixelsPerSide = float(blurSize / 2.0);

    vec2 blurMultiplyVec = (0 == horizontalPass) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec2 invTexOffset = 1.0 / texOffset;

    // Incremental Gaussian Coefficent Calculation (See GPU Gems 3 pp. 877 - 889)
    vec3 incrementalGaussian;
    incrementalGaussian.x = 1.0 / (sqrt(2.0 * pi) * sigma);
    incrementalGaussian.y = exp(-0.5 / (sigma * sigma));
    incrementalGaussian.z = incrementalGaussian.y * incrementalGaussian.y;

    vec4 avgValue = vec4(0.0, 0.0, 0.0, 0.0);
    float coefficientSum = 0.0;

    // Take the central sample first...
    avgValue += texture2D(tex0, vertTexCoord.st) * incrementalGaussian.x;
    coefficientSum += incrementalGaussian.x;
    incrementalGaussian.xy *= incrementalGaussian.yz;

    // Go through the remaining 8 vertical samples (4 on each side of the center)
    for (float i = 1.0; i <= numBlurPixelsPerSide; i++) {
        avgValue += texture2D(tex0, vertTexCoord.st - i * invTexOffset * blurMultiplyVec) * incrementalGaussian.x;
        avgValue += texture2D(tex0, vertTexCoord.st + i * invTexOffset * blurMultiplyVec) * incrementalGaussian.x;
        coefficientSum += 2.0 * incrementalGaussian.x;
        incrementalGaussian.xy *= incrementalGaussian.yz;
    }

    oColor = avgValue / coefficientSum;
	oColor = vec4(1.0, 1.0, 1.0, oColor.a);
}
)FRAG";


const std::string shadowVert = R"VERT(
#version 150
uniform mat4       ciModelMatrix;
uniform mat4       ciModelViewProjection;
in vec4            ciPosition;
in vec2            ciTexCoord0;
in vec4            ciColor;
out vec2           TexCoord0;
out vec4           Color;

void main()
{
    gl_Position = ciModelViewProjection * ciPosition;
    TexCoord0 = ciTexCoord0;
    Color = ciColor;
}
)VERT";

ci::gl::GlslProgRef sBlurShader; // = ci::gl::GlslProg::create(shadowVert, shadowFrag);
ci::gl::GlslProgRef sDrawShader; // = ci::gl::GlslProg::create(vertShader, opacityFrag);

char BLOB_TYPE = 0;

const ds::ui::DirtyState& DIRTYSHADOW = ds::ui::INTERNAL_A_DIRTY;

const char SHADOW_ATT = 80;

} // namespace

namespace waffles {
	using namespace sub_waffles;
void ShadowLayout::installAsServer(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](ds::BlobReader& r) { ds::ui::Sprite::handleBlobFromClient(r); });
}

void ShadowLayout::installAsClient(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](ds::BlobReader& r) { ds::ui::Sprite::handleBlobFromServer<ShadowLayout>(r); });
}

ShadowLayout::ShadowLayout(ds::ui::SpriteEngine& g)
	: ds::ui::LayoutSprite(g)
	, mShadowColor(0.0f, 0.0f, 0.0f)
	, mIterations(2)
	, mShadowOpacity(1.0)
	, mShadowOffset(0.0f, 0.0f)
	, mBlurShader(nullptr)
	, mBlurStart(0) 
	, mBlurTarget(0)
	, mPaddedSize(0) {

	mBlobType = BLOB_TYPE;

	mShadowSize	   = mEngine.getAppSettings().getInt("shadow:default_size", 0, 32);
	mShadowSigma   = mEngine.getAppSettings().getFloat("shadow:default_sigma", 0, 3.0f);
	mShadowOffset  = mEngine.getAppSettings().getVec2("shadow:default_offset", 0, ci::vec2());
	mShadowOpacity = mEngine.getAppSettings().getFloat("shadow:default_opacity", 0, 1.0f);
	mShadowScale   = mEngine.getAppSettings().getFloat("shadow:default_scale", 0, 1.0f);
	mIterations	   = mEngine.getAppSettings().getInt("shadow:default_iterations", 0, 3);
	mShadowColor =
		mEngine.getColors().getColorFromName(mEngine.getAppSettings().getString("shadow:default_color", 0, "black"));

	if (!sBlurShader || !sDrawShader) {
		try {
			mBlurShader = ci::gl::GlslProg::create(shadowVert, shadowFrag);
			mBlurShader->uniform("tex0", 0);
			mDrawShader = ci::gl::GlslProg::create(vertShader, opacityFrag);
		} catch (std::exception& e) {
			DS_LOG_WARNING("Couldn't load shadow blur shader! " << e.what());
		}
	}

	markAsDirty(DIRTYSHADOW);
	setLayoutUpdatedFunction([this] { onSizeChanged(); });
}

void ShadowLayout::setShadowSize(int ammount) {
	if (ammount == mShadowSize) return;

	mShadowSize = ammount;
	mBlurDirty	= true;
	markAsDirty(DIRTYSHADOW);
}

void ShadowLayout::setShadowIterations(int iterations) {
	if (iterations == mIterations) return;

	mIterations = iterations;
	mBlurDirty	= true;
	markAsDirty(DIRTYSHADOW);
}

void ShadowLayout::setShadowOpacity(float ammount) {
	if (ammount == mShadowOpacity) return;

	mShadowOpacity = ammount;
	mBlurDirty	   = true;
	markAsDirty(DIRTYSHADOW);
}

void ShadowLayout::setShadowSigma(float theSigma) {
	if (theSigma == mShadowSigma) return;

	mShadowSigma = theSigma;
	mBlurDirty	 = true;
	markAsDirty(DIRTYSHADOW);
}

void ShadowLayout::setShadowColor(ci::Color shadowColor) {
	if (shadowColor == mShadowColor) return;

	mShadowColor = shadowColor;
	markAsDirty(DIRTYSHADOW);
}

void ShadowLayout::setShadowOffset(ci::vec2 offset) {
	if (offset == mShadowOffset) return;

	mShadowOffset = offset;
	markAsDirty(DIRTYSHADOW);
}

void ShadowLayout::setShadowEveryFrame(bool renderEveryFrame) {
	mAlwaysRender = renderEveryFrame;
	mBlurDirty	  = true;
}

void ShadowLayout::onSizeChanged() {
	float maxShadowDim = mEngine.getAppSettings().getFloat("shadow:max_dimension", 0, 7680.f);
	bool  canShadow	   = (getWidth() < maxShadowDim && getHeight() < maxShadowDim);

	if (mShadowEnabled && !canShadow) {
		enableShadow(false);
	} else if (!mShadowEnabled && canShadow) {
		enableShadow(true);
	}

	mBlurDirty = true;
	markAsDirty(DIRTYSHADOW);

	ds::ui::Sprite* blurSource = nullptr;

	if (getChildren().size() > 0) {
		blurSource = getChildren().front();
	}

	if (!mShadowEnabled && blurSource) {
		blurSource->setFinalRenderToTexture(false);
		return;
	}
}

void ShadowLayout::onScaleChanged() {
	/* mBlurDirty = true;
	markAsDirty(DIRTYSHADOW); */
}

void ShadowLayout::onAppearanceChanged(bool visibile) {
	mBlurDirty = true;
	markAsDirty(DIRTYSHADOW);
}

void ShadowLayout::onPositionChanged() {
	/* mBlurDirty = true;
	markAsDirty(DIRTYSHADOW); */
}

void ShadowLayout::tweenBlur(int toValue, const float duration, const float delay, const ci::EaseFn& easing) {
	mBlurStart	  = mShadowSize;
	mBlurTarget	  = toValue;
	auto updateFn = [this] {
		mShadowSize = (int)((float)mBlurStart + (((float)mBlurTarget - (float)mBlurStart) * getNormalizedTweenValue()));
		mBlurDirty	= true;
	};
	tweenNormalized(duration, delay, easing, updateFn, updateFn);
}

void ShadowLayout::drawBlur() {
	if (!mSourceTexture || !mBlurShader) return;
	// int iw = (int)mSourceTexture->getWidth();
	// int ih = (int)mSourceTexture->getHeight();

	mPaddedSize = mShadowSize * ((mIterations + 1.f) / 4.f);
	int w		= (int)(mSourceTexture->getWidth() * mShadowScale) + (2.f * mPaddedSize * mShadowScale);
	int h		= (int)(mSourceTexture->getHeight() * mShadowScale) + (2.f * mPaddedSize * mShadowScale);
	if (w < 1 || h < 1) return;

	if (!mFbo0 || !mFbo1) {
		ci::gl::Fbo::Format theFormat;
		theFormat.disableDepth();
		theFormat.setSamples(4);
		mFbo0 = ci::gl::Fbo::create(w, h, theFormat);
		mFbo1 = ci::gl::Fbo::create(w, h, theFormat);

		auto mesh = ci::gl::VboMesh::create(ci::geom::Rect().rect(ci::Rectf(0.f, 0.f, (float)w, (float)h)));
		mBatch	  = ci::gl::Batch::create(mesh, mBlurShader);

		mBlurShader->uniform("texOffset", ci::vec2(w, h));
	} else if (mFbo0->getWidth() != w || mFbo0->getHeight() != h) {
		// FBOs should be created together so we only need to check one size
		ci::gl::Fbo::Format theFormat;
		theFormat.disableDepth();
		theFormat.setSamples(4);
		mFbo0 = ci::gl::Fbo::create(w, h, theFormat);
		mFbo1 = ci::gl::Fbo::create(w, h, theFormat);

		auto mesh = ci::gl::VboMesh::create(ci::geom::Rect().rect(ci::Rectf(0.f, 0.f, (float)w, (float)h)));
		mBatch	  = ci::gl::Batch::create(mesh, mBlurShader);

		mBlurShader->uniform("texOffset", ci::vec2(w, h));
	}


	mBlurShader->uniform("blurSize", int(float(mShadowSize) * mShadowScale));
	mBlurShader->uniform("sigma", mShadowSigma * mShadowScale);

	ci::gl::ScopedMatrices sm;
	ci::gl::ScopedViewport svp(ci::ivec2(w, h));

	ci::gl::setMatricesWindow(
		ci::ivec2(mSourceTexture->getWidth() + (2.f * mPaddedSize), mSourceTexture->getHeight() + (2.f * mPaddedSize)),
		true);
	ci::gl::color(1.0f, 1.0f, 1.0f, 1.0f);
	applyBlendingMode(ds::ui::BlendMode::FBO_IN);
	{
		// ci::gl::color(1.0f, 1.0f, 1.0f, mDrawOpacity);
		// Draw source at original size centered into fbo1
		ci::gl::ScopedFramebuffer fb(mFbo1);
		ci::gl::clear(ci::ColorA(0.0f, 0.0f, 0.0f, 0.0f));
		ci::gl::draw(mSourceTexture, ci::vec2(mPaddedSize));
	}

	ci::gl::setMatricesWindow(ci::ivec2(w, h), true);
	applyBlendingMode(ds::ui::BlendMode::FBO_OUT);
	for (int i = 0; i < mIterations; ++i) {
		{
			ci::gl::ScopedFramebuffer fb(mFbo0);
			ci::gl::ScopedTextureBind scopedTex(mFbo1->getColorTexture(), (uint8_t)0);
			ci::gl::clear(ci::ColorA(0.0f, 0.0f, 0.0f, 0.0f));

			// Then do the horizontal pass
			mBlurShader->uniform("horizontalPass", 1);
			mBatch->draw();
		}

		{
			ci::gl::ScopedFramebuffer fb(mFbo1);
			ci::gl::ScopedTextureBind scopedTex(mFbo0->getColorTexture(), (uint8_t)0);
			ci::gl::clear(ci::ColorA(0.0f, 0.0f, 0.0f, 0.0f));

			// Then do the vertical pass
			mBlurShader->uniform("horizontalPass", 0);
			mBatch->draw();
		}
	}

	mBlurTexture = mFbo1->getColorTexture();
	// mFbo1.reset();
}

void ShadowLayout::drawClient(const ci::mat4& transformMatrix, const ds::DrawParams& drawParams) {
	if (!visible() || getOpacity() * drawParams.mParentOpacity < std::numeric_limits<float>::epsilon()) return;


	auto children = getChildren();
	if (children.size() > 1 || children.size() == 0) {
		if (!mWarnedAboutYourChildren) {
			DS_LOG_WARNING("ShadowLayout: Only add 1 child to ShadowLayout! You have: " << children.size());
			mWarnedAboutYourChildren = true;
		}

		return;
	}

	auto blurSource		= children.front();
	auto newDrawOpacity = mOpacity * drawParams.mParentOpacity;
	mBlurDirty |= mAlwaysRender;
	if (newDrawOpacity != mDrawOpacity || mSourceOpacity != blurSource->getOpacity()) {
		mBlurDirty	   = true;
		mDrawOpacity   = newDrawOpacity;
		mSourceOpacity = blurSource->getOpacity();
		mPendingBlurs  = 4;
	}

	if (!mShadowEnabled && blurSource) {
		blurSource->setFinalRenderToTexture(false);
		ds::ui::LayoutSprite::drawClient(transformMatrix, drawParams);
		return;
	}

	// const auto isClient			 = (mEngine.getMode() == ds::ui::SpriteEngine::CLIENT_MODE);
	// const auto clientRedrawFrame = true; //(ci::app::getElapsedFrames() % 4 == this->getId() % 4);

	if (mBlurDirty || (mPendingBlurs > 0 && ci::app::getElapsedFrames() % 3 == this->getId() % 3)) {
		mPendingBlurs--;
		auto info	= ds::ui::Sprite::FinalRenderInfo();
		info.format = ci::gl::Fbo::Format();
		info.format.setSamples(4);
		auto colFmt = info.format.getColorTextureFormat();
		colFmt.mipmap(mEngine.getAppSettings().getBool("shadow:mipmap", 0, false));
		info.format.setColorTextureFormat(colFmt);
		blurSource->setFinalRenderToTexture(true, info);

		// HACK!!
		// If the source is inside a clipping sprite we need to disable during drawing, otherwise we get no texture
		// The confusingly named disableClipping will reset the prior clipping state
		ds::ui::clip_plane::enableClipping(-100000.f, -100000.f, 100000.f, 100000.f);
		auto dp = ds::DrawParams();
		// dp.mClippingParent = dp.mClippingParent;
		dp.mParentOpacity = mDrawOpacity;
		auto saveBlend	  = blurSource->getBlendMode();
		blurSource->setBlendMode(ds::ui::BlendMode::FBO_IN);
		blurSource->drawClient(ci::mat4(), dp);
		ds::ui::clip_plane::disableClipping();
		blurSource->setBlendMode(saveBlend);
		mSourceTexture = blurSource->getFinalOutTexture();

		if (!mSourceTexture) return;


		drawBlur();
		mBlurDirty = mAlwaysRender; // True if rendering shadows every frame, false otherwise

		if (!mBlurDirty && mPendingBlurs < 0) {
			blurSource->setFinalRenderToTexture(false);
		}
	}


	if (!mDrawShader || !mBlurTexture || !mSourceTexture) return;

	buildTransform();
	ci::mat4 totalTransformation = transformMatrix * mTransformation;
	ci::gl::pushModelMatrix();
	ci::gl::multModelMatrix(totalTransformation);

	ci::gl::ScopedGlslProg sGlsl(mDrawShader);
	ds::ui::clip_plane::passClipPlanesToShader(mDrawShader);

	if (mBlurTexture) {
		ci::gl::enableAlphaBlending();
		applyBlendingMode(getBlendMode());

		const auto finalOffset = mShadowOffset - ci::vec2(mPaddedSize);
		ci::gl::translate(finalOffset);
		ci::gl::scale(ci::vec2(1.f / mShadowScale));
		ci::gl::color(mShadowColor.r, mShadowColor.g, mShadowColor.b, mShadowOpacity * mDrawOpacity);
		ci::gl::ScopedTextureBind scopedTexture(mBlurTexture);
		ci::gl::drawSolidRect(ci::Rectf(0.0f, 0.0f, (float)mBlurTexture->getWidth(), (float)mBlurTexture->getHeight()));
		ci::gl::scale(ci::vec2(mShadowScale));
		ci::gl::translate(-finalOffset);
	}

	// Either draw the source texture (while we're still drawing the blurs)
	// OR draw use the traditional draw pipeline to draw the source directly over the shadow
	if (blurSource && !blurSource->isFinalRenderToTexture()) {
		ci::gl::popModelMatrix();
		ds::ui::LayoutSprite::drawClient(transformMatrix, drawParams);
	} else if (mBlurTexture) {
		applyBlendingMode(ds::ui::BlendMode::FBO_OUT);
		ci::gl::color(1.0f, 1.0f, 1.0f, mDrawOpacity * blurSource->getOpacity());
		ci::gl::ScopedTextureBind scopedTexture(mSourceTexture);
		ci::gl::drawSolidRect(
			ci::Rectf(0.0f, 0.0f, (float)mSourceTexture->getWidth(), (float)mSourceTexture->getHeight()));
		ci::gl::popModelMatrix();
	}
}

void ShadowLayout::writeAttributesTo(ds::DataBuffer& buf) {
	ds::ui::LayoutSprite::writeAttributesTo(buf);

	if (mDirty.has(DIRTYSHADOW)) {
		buf.add(SHADOW_ATT);
		buf.add(mShadowColor);
		buf.add(mShadowSize);
		buf.add(mShadowSigma);
		buf.add(mShadowOpacity);
		buf.add(mShadowOffset);
	}
}

void ShadowLayout::readAttributeFrom(const char attributeId, ds::DataBuffer& buf) {
	if (attributeId == SHADOW_ATT) {
		mShadowColor   = buf.read<ci::Color>();
		mShadowSize	   = buf.read<int>();
		mShadowSigma   = buf.read<float>();
		mShadowOpacity = buf.read<float>();
		mShadowOffset  = buf.read<ci::vec2>();
		mBlurDirty	   = true;
	} else {
		ds::ui::LayoutSprite::readAttributeFrom(attributeId, buf);
	}
}

} // namespace waffles
