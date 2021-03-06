cmake_minimum_required(VERSION 3.10)

# Vulkan4D Project Build Template 0.7
###########################################################################################
# Project Setup

	project(v4d_demo_project)
	set(APP_NAME demo)

###########################################################################################
# Configuration

# Options
option(COMPILE_APP "Build Demo/Project" ON)
option(COMPILE_TESTS "Build Unit Tests" ON)
option(COMPILE_GLFW "Compile GLFW from source instead of linking with installed library" ON)
option(COMPILE_V4D_CORE "Compile Vulkan4D Core library from source" ON)
option(COMPILE_V4D_MODULES "Compile all Vulkan4D Modules from source" ON)
option(COMPILE_ASSETS "Compile all assets" ON)
option(MODULES_LINK_WITH_V4D_CORE "Link all modules with v4d.dll/v4d.so" ON)
option(USE_CCACHE "Use ccache" ON)
option(ADDRESS_SANITIZER "Enable Address Sanitizer" OFF)

# Load Global CMAKE configuration
set(V4D_PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(V4D_PROJECT_BUILD_DIR "${V4D_PROJECT_DIR}/build")
include("${V4D_PROJECT_DIR}/src/v4d/core/global_build_config.cmake")

# Address Sanitizer
if(ADDRESS_SANITIZER)
	if(WIN32)
	else()
		string(APPEND CMAKE_CXX_FLAGS_DEBUG " -fno-omit-frame-pointer -fsanitize=address")
	endif()
endif()

# CCACHE
if(USE_CCACHE)
	find_program(CCACHE_FOUND ccache)
	if(CCACHE_FOUND)
		set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
	endif(CCACHE_FOUND)
endif()


###########################################################################################
# Builds


# GLFW
if(COMPILE_GLFW)
	set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
	set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
	set(GLFW_VULKAN_STATIC OFF CACHE BOOL "" FORCE)
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/v4d/xvk/glfw")
	target_compile_definitions(glfw PRIVATE UNICODE WINVER=0x0A00)
else()
	find_package(glfw3 3.3.2 REQUIRED)
endif()


# V4D Core
if(COMPILE_V4D_CORE)
	add_subdirectory(src/v4d/core)
endif()


# V4D Modules
if(COMPILE_V4D_MODULES)
	add_subdirectory(src/v4d/modules)
endif()


# Assets
if(COMPILE_ASSETS)
	add_subdirectory(tools/assetcompiler)
endif()


# Demo / Project
if(COMPILE_APP)
	file(GLOB V4DProjectSourceFiles "${PROJECT_SOURCE_DIR}/src/*.cpp")
	add_executable(${APP_NAME}
		"${V4DProjectSourceFiles}"
	)
	target_link_libraries(${APP_NAME}
		v4d
	)
	target_compile_definitions(${APP_NAME}
		PRIVATE -D_V4D_APP
	)
	set_target_properties(${APP_NAME}
		PROPERTIES
			COMPILE_FLAGS ${BUILD_FLAGS}
			LINK_FLAGS "-Wl,-rpath,./"
	)
endif()


# Unit Tests
if(COMPILE_TESTS)
	add_executable(tests
		"${PROJECT_SOURCE_DIR}/src/tests.cxx"
	)
	target_link_libraries(tests
		v4d
	)
	target_compile_definitions(tests
		PRIVATE -D_V4D_APP -D_V4D_TESTS
	)
	set_target_properties(tests 
		PROPERTIES 
			COMPILE_FLAGS ${BUILD_FLAGS}
			LINK_FLAGS "-Wl,-rpath,./"
	)
endif()
