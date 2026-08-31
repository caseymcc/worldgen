#!/usr/bin/env bash
# Shared by configure.sh, build.sh and run.sh. Works out where a build of a given architecture,
# platform and type belongs, and settles on a toolchain. Sourced, never run on its own.

set -euo pipefail

WORLDGEN_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# Defaults, each overridable by the flags the scripts parse or by the environment.
: "${BUILD_TYPE:=release}"
: "${BUILD_ARCH:=}"
: "${BUILD_PLATFORM:=}"
: "${BUILD_GENERATOR:=}"

worldgen_detect_arch()
{
    case "$(uname -m)" in
        x86_64|amd64)   echo "x64" ;;
        aarch64|arm64)  echo "arm64" ;;
        armv7l)         echo "arm" ;;
        i386|i686)      echo "x86" ;;
        *)              uname -m ;;
    esac
}

worldgen_detect_platform()
{
    case "$(uname -s)" in
        Linux)              echo "linux" ;;
        Darwin)             echo "macos" ;;
        MINGW*|MSYS*|CYGWIN*) echo "windows" ;;
        *)                  uname -s | tr '[:upper:]' '[:lower:]' ;;
    esac
}

# CMAKE_BUILD_TYPE wants Debug/Release; the directory is lower case.
worldgen_cmake_build_type()
{
    case "$1" in
        debug)          echo "Debug" ;;
        release)        echo "Release" ;;
        relwithdebinfo) echo "RelWithDebInfo" ;;
        minsizerel)     echo "MinSizeRel" ;;
        *)              echo "$1" ;;
    esac
}

worldgen_resolve()
{
    BUILD_ARCH="${BUILD_ARCH:-$(worldgen_detect_arch)}"
    BUILD_PLATFORM="${BUILD_PLATFORM:-$(worldgen_detect_platform)}"
    BUILD_TYPE="$(echo "${BUILD_TYPE}" | tr '[:upper:]' '[:lower:]')"

    CMAKE_BUILD_TYPE="$(worldgen_cmake_build_type "${BUILD_TYPE}")"

    # build/<arch>/<platform>/<build type>, so several configurations coexist and none of them
    # collide with the stale tree checked in at build/ itself.
    BUILD_DIR="${WORLDGEN_ROOT}/build/${BUILD_ARCH}/${BUILD_PLATFORM}/${BUILD_TYPE}"

    if [ -z "${BUILD_GENERATOR}" ]; then
        if command -v ninja >/dev/null 2>&1; then
            BUILD_GENERATOR="Ninja"
        else
            BUILD_GENERATOR="Unix Makefiles"
        fi
    fi

    export WORLDGEN_ROOT BUILD_ARCH BUILD_PLATFORM BUILD_TYPE CMAKE_BUILD_TYPE BUILD_DIR BUILD_GENERATOR
}

worldgen_check_vcpkg()
{
    if [ -n "${VCPKG_ROOT:-}" ] && [ -f "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" ]; then
        return 0
    fi

    cat >&2 <<'EOF'
error: VCPKG_ROOT is not set, or does not point at a vcpkg checkout.

CMakeLists.txt picks up $VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake automatically, so either:

    export VCPKG_ROOT=/path/to/vcpkg

or build in the container, which brings its own:

    ./scripts/docker.sh ./scripts/build.sh
EOF
    return 1
}

worldgen_check_submodules()
{
    if [ ! -f "${WORLDGEN_ROOT}/deps/fastnoise2/CMakeLists.txt" ] \
        || [ ! -f "${WORLDGEN_ROOT}/deps/imglib/CMakeLists.txt" ]; then
        echo "error: submodules are missing. Run: git submodule update --init --recursive" >&2
        return 1
    fi
}

# Consumes the flags every script shares and leaves the rest in WORLDGEN_ARGS for the caller to
# hand to cmake.
worldgen_parse_args()
{
    WORLDGEN_ARGS=()

    while [ $# -gt 0 ]; do
        case "$1" in
            -t|--type)      BUILD_TYPE="$2"; shift 2 ;;
            --debug)        BUILD_TYPE="debug"; shift ;;
            --release)      BUILD_TYPE="release"; shift ;;
            -a|--arch)      BUILD_ARCH="$2"; shift 2 ;;
            -p|--platform)  BUILD_PLATFORM="$2"; shift 2 ;;
            -g|--generator) BUILD_GENERATOR="$2"; shift 2 ;;
            --)             shift; WORLDGEN_ARGS+=("$@"); break ;;
            *)              WORLDGEN_ARGS+=("$1"); shift ;;
        esac
    done
}
