#include "ds/ui/service/pango_font_service.h"

#include "ds/app/environment.h"
#include "ds/debug/logger.h"

#include <algorithm>

#include "fontconfig/fontconfig.h"
#include "pango/pangocairo.h"

namespace {
const ds::BitMask	PANGO_FONT_LOG_M = ds::Logger::newModule("pango_font");


void setTextRenderer() {
	return;
	// If we're having problems rendering, we can try win32.
	//std::string rendererName = "win32";

	std::string rendererName = "fontconfig";
	
	int status = _putenv_s("PANGOCAIRO_BACKEND", rendererName.c_str());

	if(status == 0) {
		DS_LOG_INFO("Set Pango Cairo backend renderer to: " << rendererName);
	} else {
		DS_LOG_WARNING("Error setting Pango Cairo backend environment variable.");
	}
	
}

/*
TextRenderer getTextRenderer() {
	const char *rendererName = std::getenv("PANGOCAIRO_BACKEND");

	if(rendererName == nullptr) {
		DS_LOG_WARNING("Could not read Pango Cairo backend environment variable. Assuming native renderer.");
		return TextRenderer::PLATFORM_NATIVE;
	}

	std::string rendererNameString(rendererName);

	if((rendererNameString == "win32") || (rendererNameString == "coretext")) {
		return TextRenderer::PLATFORM_NATIVE;
	}

	if((rendererNameString == "fontconfig") || (rendererNameString == "fc")) {
		return TextRenderer::FREETYPE;
	}

	DS_LOG_WARNING("Unknown Pango Cairo backend environment variable: " << rendererNameString << ". Assuming native renderer.");
	return TextRenderer::PLATFORM_NATIVE;
}
*/
}

namespace ds {
namespace ui {

PangoFontService::PangoFontService(ds::ui::SpriteEngine& eng)
	: mFontMap(nullptr)
{
	DS_LOG_INFO_M("Initializing Pango version " <<PANGO_VERSION_STRING, PANGO_FONT_LOG_M);

	setTextRenderer();
}

void PangoFontService::loadFonts(){
	if(mFontMap){
		g_object_unref(mFontMap);
		mFontMap = nullptr;
	}

	if(!mFontMap){
		mFontMap = pango_cairo_font_map_get_default();
	}

	if(!mFontMap){
		DS_LOG_WARNING_M("Font map does not exist! Pango text sprites will be empty.", PANGO_FONT_LOG_M);
		return;
	}

	mLoadedFonts.clear();
	mLoadedFamilies.clear();

	int i;
	PangoFontFamily** families;
	int n_families;

	pango_font_map_list_families(mFontMap, &families, &n_families);

	for(i = 0; i < n_families; i++) {
		PangoFontFamily *family = families[i];

		const char *family_name;
		family_name = pango_font_family_get_name(family);

		// Also interrogate individual fonts in the family
		// Useful if something isn't rendering correctly
		PangoFontFace **pFontFaces = 0;
		int numFontFaces = 0;
		pango_font_family_list_faces(family, &pFontFaces, &numFontFaces);

		DsPangoFontFamily dsFamily;
		dsFamily.mFamilyName = family_name;

		for(int f = 0; f < numFontFaces; f++) {
			PangoFontFace *face = pFontFaces[f];

			const char *face_name = pango_font_face_get_face_name(face);
			PangoFontDescription *description = pango_font_face_describe(face);
			const char *description_string = pango_font_description_to_string(description);
			PangoWeight weight = pango_font_description_get_weight(description);
			uint32_t hash = pango_font_description_hash(description);

			DsPangoFontFace dpff;
			dpff.mDecription = description_string;
			dpff.mWeight = weight;
			dpff.mFaceName = face_name;
			dpff.mHash = hash;

			mLoadedFonts[description_string] = dpff;
			dsFamily.mFaces.push_back(dpff);

			pango_font_description_free(description);
		}

		auto findy = mLoadedFamilies.find(family_name);
		if(findy != mLoadedFamilies.end()){
			DS_LOG_WARNING_M("Duplicate font families found! " << family_name, PANGO_FONT_LOG_M);
		}

		mLoadedFamilies[family_name] = dsFamily;

		g_free(pFontFaces);
		
	}

	g_free(families);
}

void PangoFontService::logFonts(const bool includeFamilies){
	if(mLoadedFonts.empty() || mLoadedFamilies.empty()){
		loadFonts();
	}

//	std::sort(mLoadedFamilies.begin(), mLoadedFamilies.end());
	//std::sort(mLoadedFonts.begin(), mLoadedFonts.end());

	DS_LOG_INFO_M("Loaded Fonts: ", PANGO_FONT_LOG_M);

	for(auto it : mLoadedFonts){
		DS_LOG_INFO_M("Settable Font: " << it.second.mDecription, PANGO_FONT_LOG_M);
	}

	if(includeFamilies){
		for(auto it : mLoadedFamilies){
			DS_LOG_INFO_M("\tFamily: " << it.second.mFamilyName, PANGO_FONT_LOG_M);
			for(auto fit : it.second.mFaces){
				DS_LOG_INFO_M("\t\tFace " << ": " << fit.mFaceName, PANGO_FONT_LOG_M);
				DS_LOG_INFO_M("\t\t\tDescription: " << fit.mDecription, PANGO_FONT_LOG_M);
				DS_LOG_INFO_M("\t\t\tWeight: " << fit.mWeight, PANGO_FONT_LOG_M);
				DS_LOG_INFO_M("\t\t\tHash: " << fit.mHash, PANGO_FONT_LOG_M);
			}
		}
	}
}

bool PangoFontService::getFamilyExists(const std::string familyName){
	return mLoadedFamilies.find(familyName) != mLoadedFamilies.end();
}

bool PangoFontService::getFaceExists(const std::string faceName){
	return mLoadedFonts.find(faceName) != mLoadedFonts.end();
}

PangoFontMap* PangoFontService::getPangoFontMap(){
	return mFontMap;
}

} // namespace ui
} // namespace ds