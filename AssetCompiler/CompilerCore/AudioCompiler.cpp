/*
* @file AudioCompiler.cpp
* @brief Implementation of audio compilation
* @details Handles audio loading, processing, and binary serialization for runtime playback
* @author
* @date October 2025
*/

#include "AudioCompiler.h"
#include "../Utility/DescriptorParser.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstdarg>
#include <algorithm>
#include <cmath>

#include "../rapidjson/document.h"
#include "../rapidjson/istreamwrapper.h"

