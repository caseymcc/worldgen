cmake_minimum_required (VERSION 3.12)

option(WORLDGEN_MAPGEN "Build test app" ON)

##########################################
#HUNTER caching
set(HUNTER_STATUS_DEBUG ON)
set(HUNTER_KEEP_PACKAGE_SOURCES ON)
set(
    HUNTER_CACHE_SERVERS
    "https://github.com/huntercache/voxigen"
    CACHE
    STRING
    "Default cache server"
)

string(COMPARE EQUAL "$ENV{TRAVIS}" "true" is_travis)
string(COMPARE EQUAL "$ENV{APPVEYOR}" "True" is_appveyor)
string(COMPARE EQUAL "$ENV{GITHUB_USER_PASSWORD}" "" password_is_empty)

if((is_travis OR is_appveyor) AND NOT password_is_empty)
  option(HUNTER_RUN_UPLOAD "Upload cache binaries" ON)
endif()
message(STATUS "Hunter upload: ${HUNTER_RUN_UPLOAD}")
set(
    HUNTER_PASSWORDS_PATH
    "${CMAKE_CURRENT_LIST_DIR}/cmake/Hunter/passwords.cmake"
    CACHE
    FILEPATH
    "Hunter passwords"
)
##########################################

include(${CMAKE_CURRENT_LIST_DIR}/cmake/Hunter/HunterGate.cmake)

HunterGate(
    URL "https://github.com/cpp-pm/hunter/archive/v0.23.267.tar.gz"
    SHA1 "9c5c7fa6e17c2ae15dd922184bc7c51235aaae70"
    LOCAL
)

project(worldgen VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#variable to accumalte all target locations for debug info
set(workgen_library_targets
)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/targetInfo.cmake)

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

add_subdirectory(deps)

hunter_add_package(glm)
find_package(glm CONFIG REQUIRED)
hunter_add_package(RapidJSON)
find_package(RapidJSON CONFIG REQUIRED)

set(worldgen_source
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
#utils
    source/utils/colorMap.cpp
    include/worldgen/utils/colorMap.h
    source/utils/randomcolor.cpp
    include/worldgen/utils/randomcolor.h
)

set(worldgen_libraries
    glm
    RapidJSON::rapidjson
    FastNoise
)

source_group("source" FILES ${worldgen_source})

add_library(worldgen
    ${worldgen_source}
)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    target_compile_options(worldgen PUBLIC "-lstdc++fs")
    target_link_libraries(worldgen PUBLIC stdc++fs)
endif()

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(worldgen PUBLIC ${worldgen_libraries})

if(WORLDGEN_MAPGEN)
    hunter_add_package(imgui)
    find_package(imgui CONFIG REQUIRED)
    hunter_add_package(glfw)
    find_package(glfw3 REQUIRED)

    hunter_add_package(glbinding)
    find_package(glbinding CONFIG REQUIRED)

    hunter_add_package(CreateLaunchers)
    find_package(CreateLaunchers CONFIG REQUIRED)
    
    include(CreateLaunchers)

    set(worldgen_mapgen_sources
       
       mapgen/initGlew.cpp
       mapgen/initGlew.h
       mapgen/main.cpp
       mapgen/mapgen.cpp
       mapgen/mapgen.h
    )

    source_group("source" FILES ${worldgen_mapgen_sources})
    
    add_executable(mapgen ${worldgen_mapgen_sources})

    target_include_directories(mapgen PRIVATE ${CMAKE_CURRENT_LIST_DIR})
    target_link_libraries(mapgen worldgen 
        glfw 
        imgui::imgui 
        glbinding::glbinding
        imglib
    )

    set(mapgen_vs_enviroment_dir)
    get_target_link_directories(mapgen_vs_enviroment_dir ${mapgen_public_libraries})
    set(mapgen_vs_working_dir ${CMAKE_CURRENT_LIST_DIR})
    
    create_target_launcher(mapgen
        RUNTIME_LIBRARY_DIRS "${mapgen_vs_enviroment_dir}"
        WORKING_DIRECTORY "${mapgen_vs_working_dir}"
    )
endif()

