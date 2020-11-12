
set(FASTNOISE2_NOISETOOL OFF CACHE BOOL "Build Noise Tool" FORCE)
add_subdirectory(fastnoise2)

set(IMGLIB_USE_COMPILETIME_TYPE OFF CACHE BOOL "Turn on/off support for compile time type informat" FORCE)
set(IMGLIB_USE_EIGEN OFF CACHE BOOL "Turn on/off support for eigen library" FORCE)
set(IMGLIB_USE_OPENCL OFF CACHE BOOL "Turn on/off support for OpenCl" FORCE)
set(IMGLIB_USE_OPENCL_CPP OFF CACHE BOOL "Turn on/off support for OpenCl CPP classes" FORCE)
set(IMGLIB_USE_CUDA OFF CACHE BOOL "Turn on/off support for CUDA" FORCE)
add_subdirectory(imglib)