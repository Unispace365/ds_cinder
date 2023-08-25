#include "stdafx.h"

#include "blend.h"
#include <algorithm>
#include <cinder/gl/gl.h>

namespace ds { namespace ui {


	void applyBlendingMode(const BlendMode& blendMode) {
		if (blendMode == NORMAL) {
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		} else if (blendMode == PREMULTIPLY) {
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		} else if (blendMode == FBO_IN) {
			glBlendEquation(GL_FUNC_ADD);
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		} else if (blendMode == FBO_OUT) {
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		} else if (blendMode == MULTIPLY) {
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
		} else if (blendMode == SCREEN) {
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
		} else if (blendMode == ADD) {
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		} else if (blendMode == SUBTRACT) {
			glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
			glBlendFunc(GL_ONE, GL_ONE);
		} else if (blendMode == LIGHTEN) {
			glBlendEquation(GL_MAX);
			glBlendFunc(GL_ONE, GL_ONE);
		} else if (blendMode == DARKEN) {
			glBlendEquation(GL_MIN);
			glBlendFunc(GL_ONE, GL_ONE);
		} else if (blendMode == TRANSPARENT_BLACK) {
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		} else if (blendMode == EXCLUSION) {
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR);
		}
	}


	ds::ui::BlendMode getBlendModeByString(const std::string& blendString) {
		if (blendString.empty()) return NORMAL;
		std::string lowerString = blendString;
		std::transform(lowerString.begin(), lowerString.end(), lowerString.begin(), ::tolower);
		if (lowerString == "add") {
			return ADD;
		} else if (lowerString == "premultiply") {
			return PREMULTIPLY;
		} else if (lowerString == "fbo_in") {
			return FBO_IN;
		} else if (lowerString == "fbo_out") {
			return FBO_OUT;
		} else if (lowerString == "subtract") {
			return SUBTRACT;
		} else if (lowerString == "multiply") {
			return MULTIPLY;
		} else if (lowerString == "normal") {
			return NORMAL;
		} else if (lowerString == "lighten") {
			return LIGHTEN;
		} else if (lowerString == "darken") {
			return DARKEN;
		} else if (lowerString == "transparent_black") {
			return TRANSPARENT_BLACK;
		} else if (lowerString == "screen") {
			return SCREEN;
		} else if (lowerString == "exclusion") {
			return EXCLUSION;
		}

		return NORMAL;
	}

	const std::string getStringForBlendMode(const BlendMode& blendMode) {
		if (blendMode == NORMAL) {
			return "normal";
		} else if (blendMode == PREMULTIPLY) {
			return "premultiply";
		} else if (blendMode == FBO_IN) {
			return "fbo_in";
		} else if (blendMode == FBO_OUT) {
			return "fbo_out";
		} else if (blendMode == MULTIPLY) {
			return "multiply";
		} else if (blendMode == SCREEN) {
			return "screen";
		} else if (blendMode == ADD) {
			return "add";
		} else if (blendMode == SUBTRACT) {
			return "subtract";
		} else if (blendMode == LIGHTEN) {
			return "lighten";
		} else if (blendMode == DARKEN) {
			return "darken";
		} else if (blendMode == TRANSPARENT_BLACK) {
			return "transparent_black";
		} else if (blendMode == EXCLUSION) {
			return "exclusion";
		}

		return "normal";
	}

	bool premultiplyAlpha(const BlendMode& blendMode) {
		if (blendMode == NORMAL) {
			return false;
		} else if (blendMode == PREMULTIPLY) {
			return true;
		} else if (blendMode == FBO_IN) {
			return false;
		} else if (blendMode == FBO_OUT) {
			return false;
		} else if (blendMode == MULTIPLY) {
			return true;
		} else if (blendMode == SCREEN) {
			return true;
		} else if (blendMode == ADD) {
			return true;
		} else if (blendMode == SUBTRACT) {
			return true;
		} else if (blendMode == LIGHTEN) {
			return true;
		} else if (blendMode == DARKEN) {
			return true;
		} else if (blendMode == TRANSPARENT_BLACK) {
			return true;
		} else if (blendMode == EXCLUSION) {
			return true;
		}
		return false;
	}

}} // namespace ds::ui
