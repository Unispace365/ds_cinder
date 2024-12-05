/*
Copyright (c) 2023, Paul Houx Creative Coding - All rights reserved.
This code is intended for use with the Cinder C++ library: http://libcinder.org

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

	* Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#include "stdafx.h"

#include "nvpath/NvPathText.h"

#include <cairo-svg.h>
#include <cairo.h>
#include <fontconfig/fontconfig.h>
#include <pango/pango-font.h>
#include <pango/pangocairo.h>

namespace nvpath {

Text::Text(ds::ui::SpriteEngine& engine)
  : ds::ui::Text(engine) {
	enableMultiTouch(ds::ui::MULTITOUCH_CAN_POSITION | ds::ui::MULTITOUCH_CAN_ROTATE | ds::ui::MULTITOUCH_CAN_SCALE);
	enable(true);
}

void Text::onBuildRenderBatch() {}

void Text::drawLocalClient() {
	renderPangoText();

	if (mSvg) {
		ci::gl::ScopedModelMatrix sm;
		ci::gl::translate(mRenderOffset);

		ScopedPathRendering sp;

		svg::Style style;
		style.setOpacity(getDrawOpacity());

		svg::Svg renderer(mSvg);
		mSvg->setStyle(style);
		mSvg->render(renderer);
	}
}

void Text::renderPangoText() {
	if (mNeedsTextRender && mPixelWidth > 0 && mPixelHeight > 0) {
		mBuffer.clear();

		auto writeFn = [](void* closure, const unsigned char* data, unsigned int length) -> cairo_status_t {
			auto self = static_cast<Text*>(closure);
			self->mBuffer.append((const char*)data, length);
			return CAIRO_STATUS_SUCCESS;
		};

		cairo_surface_t* cairoSurface = cairo_svg_surface_create_for_stream(writeFn, this, mPixelWidth, mPixelHeight);

		auto cairoSurfaceStatus = cairo_surface_status(cairoSurface);
		if (CAIRO_STATUS_SUCCESS != cairoSurfaceStatus) {
			DS_LOG_WARNING("Error creating Cairo surface. Status:" << cairoSurfaceStatus << " w:" << mPixelWidth
																   << " h:" << mPixelHeight << " text:" << mText);
			// make sure we don't render garbage
			if (mTexture) {
				mTexture = nullptr;
			}
			return;
		}

		cairo_t* cairoContext = nullptr;
		if (cairoSurface) {
			// Create context
			cairoContext = cairo_create(cairoSurface);

			auto cairoStatus = cairo_status(cairoContext);

			if (CAIRO_STATUS_NO_MEMORY == cairoStatus) {
				DS_LOG_WARNING("Out of memory, error creating Cairo context");
				cairo_surface_destroy(cairoSurface);
				return;
			}

			if (CAIRO_STATUS_SUCCESS != cairoStatus) {
				DS_LOG_WARNING("Error creating Cairo context " << cairoStatus);
				cairo_surface_destroy(cairoSurface);
				return;
			}
		}

		// PangoLayoutIter* iter = pango_layout_get_iter(mPangoLayout);
		// if (iter) {
		//	do {
		//		PangoRectangle ink_rect, logical_rect;
		//		pango_layout_iter_get_run_extents(iter, &ink_rect, &logical_rect);

		//		PangoLayoutRun* run = pango_layout_iter_get_run_readonly(iter);
		//		if (run) {
		//			PangoFontDescription* font_desc = pango_font_describe(run->item->analysis.font);
		//			std::string			  file_name(pango_font_description_to_filename(font_desc));
		//			std::string			  font_str(pango_font_description_to_string(font_desc));
		//			std::string			  font_family(pango_font_description_get_family(font_desc));
		//			int					  font_size(pango_font_description_get_size(font_desc));
		//			pango_font_description_free(font_desc);

		//			PangoGlyphString* glyphs = run->glyphs;
		//			if (glyphs) {
		//				for (int i = 0; i < glyphs->num_glyphs; i++) {
		//					PangoGlyphInfo* glyph_info = &glyphs->glyphs[i];
		//					printf("Glyph Index: %u, Offset X: %d, Offset Y: %d\n", glyph_info->glyph,
		//						   glyph_info->geometry.x_offset, glyph_info->geometry.y_offset);
		//				}
		//			}
		//		}
		//	} while (pango_layout_iter_next_run(iter));
		//}

		if (cairoContext) {
			// Draw the text into the buffer
			cairo_set_source_rgb(cairoContext, mStyle.mColor.r, mStyle.mColor.g, mStyle.mColor.b);

			// Move the layout into the correct position on the surface/context before drawing.
			cairo_translate(cairoContext, mPixelOffsetX, mPixelOffsetY);
			pango_cairo_update_layout(cairoContext, mPangoLayout);
			pango_cairo_show_layout(cairoContext, mPangoLayout);

			cairo_destroy(cairoContext);
		}

		if (cairoSurface) {
			cairo_surface_destroy(cairoSurface);
		}

		// ci::app::console() << mBuffer << std::endl;
		auto buffer = ci::Buffer::create(mBuffer.data(), mBuffer.size());
		auto source = ci::DataSourceBuffer::create(buffer);
		mSvg		= svg::SvgDoc::create(source);

		mNeedsTextRender = false;
	}
}

} // namespace nvpath