#ifndef _worldgen_export_h_
#define _worldgen_export_h_

#ifdef WORLDGEN_STATIC_DEFINE
#  define WORLDGEN_EXPORT
#  define WORLDGEN_NO_EXPORT
#else
#   define WORLDGEN_EXPORT 
#  ifndef WORLDGEN_EXPORT
#    ifdef WORLDGEN_EXPORTS
// We are building this library
#      define WORLDGEN_EXPORT __declspec(dllexport)
#    else
// We are using this library
#      define WORLDGEN_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef WORLDGEN_NO_EXPORT
#    define WORLDGEN_NO_EXPORT 
#  endif
#endif

#ifndef WORLDGEN_DEPRECATED
#  define WORLDGEN_DEPRECATED __declspec(deprecated)
#endif

#ifndef WORLDGEN_DEPRECATED_EXPORT
#  define WORLDGEN_DEPRECATED_EXPORT WORLDGEN_EXPORT WORLDGEN_DEPRECATED
#endif

#ifndef WORLDGEN_DEPRECATED_NO_EXPORT
#  define WORLDGEN_DEPRECATED_NO_EXPORT WORLDGEN_NO_EXPORT WORLDGEN_DEPRECATED
#endif

#if 0 // DEFINE_NO_DEPRECATED
#  ifndef WORLDGEN_NO_DEPRECATED
#    define WORLDGEN_NO_DEPRECATED
#  endif
#endif

#endif //_worldgen_export_h_