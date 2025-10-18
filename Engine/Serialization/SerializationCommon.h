#pragma once

// Standard Library
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <typeindex>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>

// GLM Math Library
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// This header should be included by all serialization files
// to ensure consistent dependencies