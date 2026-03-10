
#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <chrono>

#include <thread>
#include <algorithm>

namespace fs = std::filesystem;

static std::string getrepoRoot() {
	fs::path current = fs::current_path();

	while (!current.empty() && current != current.parent_path()) {
		if (fs::exists(current / "CMakeLists.txt")) {
			return current.generic_string();
		}
		current = current.parent_path();
	}

	//cannot get the repo root: no CMakeLists.txt, falling back into current directory
	return fs::current_path().generic_string();

}