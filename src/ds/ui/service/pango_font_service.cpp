#include "stdafx.h"

#include "ds/ui/service/pango_font_service.h"

#include "ds/app/environment.h"
#include "ds/debug/logger.h"

#include <algorithm>
#include <ds/util/file_meta_data.h>

#include "fontconfig/fontconfig.h"
#include "pango/pangocairo.h"


namespace {
const ds::BitMask	PANGO_FONT_LOG_M = ds::Logger::newModule("pango_font");
}

namespace ds {
namespace ui {

PangoFontService::PangoFontService(ds::ui::SpriteEngine& eng)
	: mFontMap(nullptr)
{
	DS_LOG_INFO_M("Initializing Pango version " << PANGO_VERSION_STRING, PANGO_FONT_LOG_M);
	std::cout << "Initializing Pango version " << PANGO_VERSION_STRING << std::endl;

	// Note: _putenv doesn't work for successfully propagating variables to the pango / fontconfig dll's
	//		So the backend variable must be set system-wide. the setx command does that, but it won't be picked up
	//		until the app runs again. It's not the best, but it's not the worst!
	//		The backend has to be set to fontconfig for antialiasing to look good. 
	//		Otherwise it uses the windows native renderer, which looks gross
	auto envy = getenv("PANGOCAIRO_BACKEND");
	if(!envy){
		std::cout << "Pango cairo backend variable not set! Restart the app for prettier fonts" << std::endl;
		system("setx PANGOCAIRO_BACKEND fontconfig");
	}
}

void PangoFontService::loadFonts(){
	DS_LOG_INFO("Creating pango font map...");

	mFontMap = pango_cairo_font_map_get_default();
	DS_LOG_INFO("Map Created.");


//	return;

	if(!mFontMap){
		DS_LOG_WARNING_M("Font map does not exist! Pango text sprites will be empty.", PANGO_FONT_LOG_M);
		return;
	}

	mLoadedFonts.clear();
	mLoadedFamilies.clear();

	int i;
	PangoFontFamily** families = nullptr;
	int n_families = 0;

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

	DS_LOG_INFO("Pango font map loaded.");
}

bool PangoFontService::loadFont(const std::string& path, const std::string& fontName) {
	if(!ds::safeFileExistsCheck(path)) return false;
	const FcChar8 *fcPath = (const FcChar8 *)path.c_str();
	FcBool fontAddStatus = false;
	// NOTE: calling into fontconfig at this point creates a runtime error
	// Windows seems to totally fuck shit up trying to find the correct dll. 
	// When that's resolved, could actually try to load the correct fonts
	fontAddStatus = FcConfigAppFontAddFile(FcConfigGetCurrent(), fcPath);

	if(!fontAddStatus) {
		DS_LOG_WARNING_M("Pango failed to load font from file \"" << path << "\"", PANGO_FONT_LOG_M);
		return false;
	} else {
		// it's looking like every call to the FcConfigAppFontAddFile succeeds, so this is kinda pointless
		//DS_LOG_INFO_M("Pango thinks it loaded font " << path << " with status " << fontAddStatus, PANGO_FONT_LOG_M);
	}

	if(!getFaceExists(fontName)){
		DsPangoFontFace dpff;
		dpff.mDecription = "Local loaded font";
		dpff.mWeight = "400";
		dpff.mFaceName = fontName;
		mLoadedFonts[fontName] = dpff;
	}

	return true;
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

void PangoFontService::setTextSuffix(const std::wstring& suffix){
	mTextSuffix = suffix;
}

std::wstring PangoFontService::getTextSuffix(){
	return mTextSuffix;
}

} // namespace ui
} // namespace ds