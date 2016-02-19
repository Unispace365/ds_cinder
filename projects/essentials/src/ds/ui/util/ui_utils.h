#pragma once
#ifndef DS_UI_UTIL_UI_UTILS
#define DS_UI_UTIL_UI_UTILS

#include <ds/ui/sprite/sprite.h>

namespace ds {
namespace ui {

static void fitInside(Sprite* sp, const ci::Rectf area, const bool letterbox){
	if(!sp) return;
	// a = w / h;
	// h = w /a;
	// w = ah;
	const float spriteAspect = sp->getWidth() / sp->getHeight();
	const float areaAspect = area.getWidth() / area.getHeight();
	float destScale = sp->getScale().x;

	if(letterbox){
		// When letterboxing, if the sprite is narrower then the dest area, fill the height
		if(spriteAspect < areaAspect){
			destScale = area.getHeight() / sp->getHeight();
		} else {
			destScale = area.getWidth() / sp->getWidth();
		}
	} else {
		// When NOT letterboxing, if the sprite is wider then the dest area, fill the height
		if(spriteAspect > areaAspect){
			destScale = area.getHeight() / sp->getHeight();
		} else {
			destScale = area.getWidth() / sp->getWidth();
		}
	}

	sp->setScale(destScale, destScale, 1.0f);
	sp->setPosition(area.getX1() + area.getWidth() / 2.0f - sp->getScaleWidth() / 2.0f, area.getY1() + area.getHeight() / 2.0f - sp->getScaleHeight() / 2.0f);
}

typedef enum {
	kAlignUpperLeft,
	kAlignUpperCenter,
	kAlignUpperRight,
	kAlignMiddleLeft,
	kAlignMiddleCenter,
	kAlignMiddleRight,
	kAlignLowerLeft,
	kAlignLowerCenter,
	kAlignLowerRight
} AlignType;

static void insetRectInRectWithAlign(ci::Rectf& rect1, const ci::Rectf& rect2, AlignType align = ds::ui::kAlignMiddleCenter, bool preserveAspect = true, bool fillRect2 = false) {
	float w = rect1.getWidth();
	float h = rect1.getHeight();
	if(
		preserveAspect &&
		(w > 0.01f) &&
		(h > 0.01f)
	) {
		// figure out the aspect
		float newW = 0.0f;
		float newH = 0.0f;

		float targetW = rect2.getWidth();
		float targetH = rect2.getHeight();

		float aspect = 1.0f;
		float scale = 1.0f;
		if(h > 0.01f) {
			aspect = (w / h);
			scale = (targetH / h);
		}

		// now size within the extent, trying height first
		newW = w * scale;
		if(fillRect2) {
			// expand width to fit
			if((newW < targetW) && (w > 0.01f)) {
				scale = (targetW / w);
				newW = w * scale;
			}
		} else {
			// shrink width to fit
			if((newW > targetW) && (w > 0.01f)) {
				scale = (targetW / w);
				newW = w * scale;
			}
		}
		newH = h * scale;

		// we will shortly size rect1 to (newW, newH)
			
		// now align within the extent
		float x = 0.0f;
		float y = 0.0f;

		if((align == kAlignUpperCenter) || (align == kAlignMiddleCenter) || (align == kAlignLowerCenter)) {
			x = ((targetW - newW) * 0.5f);
		}
			
		if((align == kAlignUpperRight) || (align == kAlignMiddleRight) || (align == kAlignLowerRight)) {
			x = (targetW - newW);
		}

		if((align == kAlignMiddleLeft) || (align == kAlignMiddleCenter) || (align == kAlignMiddleRight)) {
			y = ((targetH - newH) * 0.5f);
		}
			
		if((align == kAlignLowerLeft) || (align == kAlignLowerCenter) || (align == kAlignLowerRight)) {
			y = (targetH - newH);
		}

		rect1.set(rect2.x1 + x, rect2.y1 + y, rect2.x1 + x + newW, rect2.y1 + y + newH);
	} else {
		// just make same size as the extent
		rect1 = rect2;
	}
}

static void ColorToHSV(const ci::Color& rgb, float* h, float* s, float* v) {
	float minRGB = fminf(rgb.r, fminf(rgb.g, rgb.b));
	float maxRGB = fmaxf(rgb.r, fmaxf(rgb.g, rgb.b));
	*v = maxRGB;
	float deltaRGB = maxRGB - minRGB;
	if(maxRGB == 0.0f) {
		*s = 0.0f;
		*h = -1.0f;
	} else {
		*s = deltaRGB / maxRGB;
		if(rgb.r == maxRGB) {
			*h = (rgb.g - rgb.b) / deltaRGB;
		} else if(rgb.g == maxRGB) {
			*h = 2.0f + (rgb.b - rgb.r) / deltaRGB;
		} else {
			*h = 4.0f + (rgb.r - rgb.g) / deltaRGB;
		}
		*h *= 60.0f;				
		if(*h < 0.0f) {
			*h += 360.0f;
		}
	}
}

static void HSVToColor(float h, float s, float v, ci::Color* rgb )
{
	float r, g, b;
	if(s == 0.0f) {
		r = g = b = v;
	} else {
		int sector = (int)floor(h / 60.0f);
		float remainder = (h - ((float)sector * 60.0f)) / 60.0f;
		float p = v * (1.0f - s);
		float q = v * (1.0f - s * remainder);
		float t = v * (1.0f - s * (1.0f - remainder));
		switch(sector) {
			case 0:
				r = v;
				g = t;
				b = p;
				break;
			case 1:
				r = q;
				g = v;
				b = p;
				break;
			case 2:
				r = p;
				g = v;
				b = t;
				break;
			case 3:
				r = p;
				g = q;
				b = v;
				break;
			case 4:
				r = t;
				g = p;
				b = v;
				break;
			default:
				r = v;
				g = p;
				b = q;
				break;
		}
	}
	rgb->set(r, g, b);
}


} // namespace ui
} // namespace ds

#endif
