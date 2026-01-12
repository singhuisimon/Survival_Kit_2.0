

#pragma once

#include "FontCompiler.h"

#include <map>
#include <algorithm>
#include <cmath>
#include <limits>


namespace AssetCompiler {

	std::vector<uint32_t> FontCompiler::getCharacterSet() {
		std::vector<uint32_t> chars;

		//printable ASCII: space(32) to tilde(126)
		for (uint32_t c = 32; c <= 126; c++) {
			chars.push_back(c);
		}

		return chars;
	}

	FontCompiler::GlyphRasterData  FontCompiler::rasterizeGlyph(FT_Face face, uint32_t charCode) {

		GlyphRasterData result;

		//set size
		
	}

}//end of namespace AssetCompiler