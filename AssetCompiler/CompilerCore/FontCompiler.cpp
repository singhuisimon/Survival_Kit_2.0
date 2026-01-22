



#include "FontCompiler.h"

#include <map>
#include <algorithm>
#include <cmath>
#include <limits>


namespace AssetCompiler {

	//static member initialization
	FT_Library FontCompiler::ftLibrary;
	bool FontCompiler::ftInitialized = false;

	std::vector<uint32_t> FontCompiler::getCharacterSet() {
		std::vector<uint32_t> chars;

		//printable ASCII: space(32) to tilde(126)
		for (uint32_t c = 32; c <= 126; c++) {
			chars.push_back(c);
		}

		return chars;
	}



}//end of namespace AssetCompiler