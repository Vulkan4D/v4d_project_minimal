#pragma once

#define ZAP_USE_REINTERPRET_CAST_INSTEAD_OF_MEMCPY
// #define EVENT_DEFINITIONS_VARIADIC

// Vulkan configuration
#define V4D_RENDERER_FRAMEBUFFERS_MAX_FRAMES 3
#define V4D_VULKAN_VALIDATION_ABORT_ON_ERROR
#define V4D_VULKAN_USE_VMA
// #define V4D_VULKAN_USE_VALIDATION_LAYERS // automatically enabled on linux in debug mode
// #define V4D_VULKAN_NO_VALIDATION_LAYERS

#define COMMON_OBJECTS_ENABLE_FLEXIBLE_PTR_OFFSET

// #define XVK_USE_QT_VULKAN_LOADER // uncomment if using Qt
#define XVK_INCLUDE_GLFW // comment if using Qt or another window context manager

// GLM Configuration
#define XVK_INCLUDE_GLM
#define GLM_FORCE_INTRINSICS
#define GLM_FORCE_SIMD_AVX2
#define GLM_FORCE_CXX17
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

// Needed for ImGui and VMA
namespace VkFunctions {}
#define XVK_EXPOSE_NATIVE_VULKAN_FUNCTIONS_NAMESPACE VkFunctions
#ifdef _V4D_CORE
	#define XVK_EXPORT
#else
	#define XVK_IMPORT
#endif

// #define V4D_INCLUDE_TINYGLTFLOADER
