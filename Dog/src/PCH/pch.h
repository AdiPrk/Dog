#pragma once

#define MAX_MODEL_COUNT 250
#define MAX_TEXTURE_COUNT 250
#define MAX_BONES 100
#define MAX_BONE_INFLUENCE 4
#define INVALID_MODEL_INDEX 9999
#define INVALID_TEXTURE_INDEX 9999

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <thread>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <unordered_set>
#include <set>
#include <unordered_map>
#include <map>
#include <regex>
#include <future>

#include "vk_mem_alloc.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

// My files
#include "Graphics/Vulkan/Models/assimpGlmHelper.h"
