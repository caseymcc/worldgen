
set(CURRENT_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_RELEASE})
add_subdirectory(fastnoise2)
set(CMAKE_CXX_FLAGS_DEBUG ${CURRENT_CXX_FLAGS_DEBUG})

set(IMGLIB_USE_COMPILETIME_TYPE OFF CACHE BOOL "Turn on/off support for compile time type informat" FORCE)
set(IMGLIB_USE_EIGEN OFF CACHE BOOL "Turn on/off support for eigen library" FORCE)
set(IMGLIB_USE_OPENCL OFF CACHE BOOL "Turn on/off support for OpenCl" FORCE)
set(IMGLIB_USE_OPENCL_CPP OFF CACHE BOOL "Turn on/off support for OpenCl CPP classes" FORCE)
set(IMGLIB_USE_CUDA OFF CACHE BOOL "Turn on/off support for CUDA" FORCE)
add_subdirectory(imglib)