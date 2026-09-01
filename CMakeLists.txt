cmake_minimum_required (VERSION 3.15)

option(WORLDGEN_MAPGEN "Build test app" ON)
option(WORLDGEN_TESTS "Build the test suite" ON)
option(WORLDGEN_SHARED "Build as shared lib" OFF)
option(WORLDGEN_FORCE_RELEASERUNTIME "Force using release runtime in debug" OFF)

##########################################
#vcpkg
#
#Dependencies are declared in vcpkg.json. Manifest features have to be selected
#before project() runs, so the mapgen-only packages are gated here.
if(WORLDGEN_MAPGEN)
    list(APPEND VCPKG_MANIFEST_FEATURES "mapgen")
endif()

#Pick the vcpkg toolchain up from VCPKG_ROOT unless the caller passed their own
#-DCMAKE_TOOLCHAIN_FILE.
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
    message(STATUS "Using vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
endif()
##########################################

project(worldgen VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#setup build folders
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/)
#for multi-config builds (e.g. msvc)
if(CMAKE_CONFIGURATION_TYPES)
    foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${PROJECT_BINARY_DIR}/bin/${OUTPUTCONFIG}/)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${PROJECT_BINARY_DIR}/lib/${OUTPUTCONFIG}/)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${PROJECT_BINARY_DIR}/lib/${OUTPUTCONFIG}/)
    endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)
else()
    #make sure there is a build type
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Debug)
    endif()
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(WORLDGEN_FORCE_RELEASERUNTIME)
    add_compile_options("/MD")
endif()

add_subdirectory(deps)

find_package(glm CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)

#RapidJSON ships upstream's config, which exports include dirs but no imported
#target. Normalise it so the rest of the file can just link a target.
if(NOT TARGET RapidJSON::rapidjson)
    add_library(RapidJSON::rapidjson INTERFACE IMPORTED)
    if(RAPIDJSON_INCLUDE_DIRS)
        set(worldgen_rapidjson_includes ${RAPIDJSON_INCLUDE_DIRS})
    else()
        set(worldgen_rapidjson_includes ${RapidJSON_INCLUDE_DIRS})
    endif()
    set_target_properties(RapidJSON::rapidjson PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${worldgen_rapidjson_includes}"
    )
endif()

set(worldgen_source
#generic
    include/generic/io/fs.h
#core
    include/worldgen/biome.h
    source/biome.cpp
    include/worldgen/export.h
    include/worldgen/fill.h
    include/worldgen/generator.h
    include/worldgen/perturbedWeather.h
    source/perturbedWeather.cpp
    include/worldgen/progress.h
    include/worldgen/sortedVector.h
    include/worldgen/tectonics.h
    include/worldgen/weather.h
    include/worldgen/worldDescriptors.h
    include/worldgen/wrap.h
#maths
    include/worldgen/maths/boundingBox.h
    source/maths/boundingBox.cpp
    include/worldgen/maths/coords.h
    source/maths/coords.cpp
    include/worldgen/maths/math_helpers.h
#generators
    include/worldgen/generators/equiRectWorldGenerator.h
    source/generators/equiRectWorldGenerator.cpp
    source/generators/detailPass.cpp
    include/worldgen/worldgen_c.h
    source/worldgen_c.cpp
#utils
    source/utils/colorMap.cpp
    include/worldgen/utils/colorMap.h
    source/utils/randomcolor.cpp
    include/worldgen/utils/randomcolor.h
)

set(worldgen_libraries
    glm::glm
    RapidJSON::rapidjson
    FastNoise
)

source_group("source" FILES ${worldgen_source})

if(WORLDGEN_SHARED)
    add_library(worldgen SHARED
        ${worldgen_source}
    )
else()
    add_library(worldgen
        ${worldgen_source}
    )
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    target_compile_options(worldgen PUBLIC "-lstdc++fs")
    target_link_libraries(worldgen PUBLIC stdc++fs)
endif()

#Hunter built glm with GLM_FORCE_CTOR_INIT for gcc; vcpkg ships a stock glm, so
#the define has to be applied here instead.
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_definitions(worldgen PUBLIC GLM_FORCE_CTOR_INIT)
endif()

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
#The library's own headers have to travel with the TARGET, not just with this directory. A bare
#include_directories() compiles worldgen's own sources and exports nothing, so anything linking
#worldgen as a library - another cmake project, or Unreal through the CMakeTarget plugin - gets the
#libraries and not the headers, and fails on the first #include <worldgen/...>.
target_include_directories(worldgen PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(worldgen PUBLIC ${CMAKE_CURRENT_LIST_DIR}/deps/easing)
target_link_libraries(worldgen PUBLIC ${worldgen_libraries})

if(WORLDGEN_FORCE_RELEASERUNTIME)
    message(STATUS "Forcing release CRT")
#    set_property(TARGET worldgen PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    if(MSVC)
        target_compile_options(worldgen PUBLIC "/MD")
    endif()
endif()

if(WORLDGEN_MAPGEN)
    find_package(imgui CONFIG REQUIRED)
    find_package(glfw3 CONFIG REQUIRED)
    find_package(glbinding CONFIG REQUIRED)
    find_package(GLEW REQUIRED)

    set(worldgen_mapgen_sources
       mapgen/chunkViewer.cpp
       mapgen/chunkViewer.h
       mapgen/imageWidget.cpp
       mapgen/imageWidget.h
       mapgen/imguiHelpers.cpp
       mapgen/imguiHelpers.h
       mapgen/imguiWidget.cpp
       mapgen/imguiWidget.h
       mapgen/infoViewer.cpp
       mapgen/infoViewer.h
       mapgen/initGlew.cpp
       mapgen/initGlew.h
       mapgen/main.cpp
       mapgen/mapgen.cpp
       mapgen/mapgen.h
       mapgen/legendViewer.cpp
       mapgen/legendViewer.h
       mapgen/legend.cpp
       mapgen/legend.h
       mapgen/terrainPalette.h
       mapgen/regionViewer.cpp
       mapgen/regionViewer.h
       mapgen/uiData.h
       mapgen/uiDataWidget.cpp
       mapgen/uiDataWidget.h
       mapgen/worldControls.cpp
       mapgen/worldControls.h
       mapgen/worldViewer.cpp
       mapgen/worldViewer.h
    )

    source_group("source" FILES ${worldgen_mapgen_sources})

    add_executable(mapgen ${worldgen_mapgen_sources})

    target_include_directories(mapgen PRIVATE ${CMAKE_CURRENT_LIST_DIR})

    target_link_libraries(mapgen worldgen
        glfw
        imgui::imgui
        glbinding::glbinding
        GLEW::GLEW
        imglib
    )

#    if(WORLDGEN_FORCE_RELEASERUNTIME)
#        if(MSVC)
#            target_compile_options(mapgen PUBLIC "/MD")
#        endif()
#    endif()

    set_target_properties(mapgen PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
    )
endif()

if(WORLDGEN_TESTS)
    #Invariants rather than expected values: the world changes whenever a parameter moves, and a
    #test that pinned an exact number would have to be edited on every tune.
    add_executable(worldgen_tests tests/worldgenTests.cpp)
    target_link_libraries(worldgen_tests worldgen)

    enable_testing()
    add_test(NAME worldgen COMMAND worldgen_tests)
endif()
