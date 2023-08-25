#pragma once
#ifndef DS_UI_SERVICE_PANGO_FONT_SERVICE_H_
#define DS_UI_SERVICE_PANGO_FONT_SERVICE_H_

#include "ds/app/engine/engine_service.h"

#include <map>
#include <vector>

struct _PangoFontMap;
typedef struct _PangoFontMap PangoFontMap;

namespace ds { namespace ui {
	class LoadImageService;
	class SpriteEngine;

	/**
	 * \class PangoFontService
	 * \brief Loads the fonts on this system and exposes them to Pango text sprites
	 */
	class PangoFontService {

	  private:
		struct DsPangoFontFace {
			/// Normal, Bold, Oblique, Demi Bold Oblique, etc
			std::string mFaceName;
			/// The full face name, such as "Gill Sans MT Bold Oblique"
			std::string mDecription;
			/// Numerical identifier of weights, such as 400 for normal, 700 for bold
			std::string mWeight;
			/// Pango-reported hash
			unsigned int mHash;
		};

		struct DsPangoFontFamily {
			/// Family name is something like Arial
			std::string mFamilyName;
			/// Faces are specific fonts inside a family
			std::vector<DsPangoFontFace> mFaces;
		};

	  public:
		PangoFontService(ds::ui::SpriteEngine& eng);

		/// Clears previously-loaded fonts and reloads the fonts installed in Windows
		void loadFonts();


		void loadFamiliesAndFaces();

		/// Load a local font file.
		/// This is called when you use FontList::installFont(), recommend you use that method instead unless you know
		/// what you're doing
		bool loadFont(const std::string& path, const std::string& fontName);

		/// Logs all the fonts loaded in Windows to std::cout and to DS_LOG_INFO
		/// If including faces, will print specific info about each face in a family
		void logFonts(const bool includeFaces);

		/// If true, the family is installed in windows and *should* work for creating pango text sprites
		bool getFamilyExists(const std::string familyName);
		/// If true, the specific face exists and is installed.
		bool getFaceExists(const std::string faceName);

		/// For creating pango contexts. Check for nullptr before using
		PangoFontMap* getPangoFontMap();

	  private:
		PangoFontMap*							 mFontMap;
		std::map<std::string, DsPangoFontFamily> mLoadedFamilies;
		std::map<std::string, DsPangoFontFace>	 mLoadedFonts;
	};

}} // namespace ds::ui

#endif
