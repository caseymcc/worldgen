#!/usr/bin/env bash
# Start the viewer. One command, from a clean checkout, on a host with nothing but docker installed:
# it builds the container image if that has not been done, builds the project if that has not been
# done, and then runs mapgen on this machine's display.
#
#   ./run.sh                  release
#   ./run.sh --debug          debug
#   ./run.sh --rebuild        build the project again even if it looks built
#   ./run.sh --rebuild-image  build the container image again as well
#   ./run.sh --native         skip docker: build and run with the host's own toolchain
#
# Everything here delegates to scripts/, which can still be driven a step at a time.

set -euo pipefail

WORLDGEN_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

BUILD_TYPE="release"
REBUILD=0
REBUILD_IMAGE=0
NATIVE=0
PASSTHROUGH=()

while [ $# -gt 0 ]; do
    case "$1" in
        -t|--type)       BUILD_TYPE="$2"; shift 2 ;;
        --debug)         BUILD_TYPE="debug"; shift ;;
        --release)       BUILD_TYPE="release"; shift ;;
        --rebuild)       REBUILD=1; shift ;;
        --rebuild-image) REBUILD_IMAGE=1; shift ;;
        --native)        NATIVE=1; shift ;;
        -h|--help)
            sed -n '2,12p' "${BASH_SOURCE[0]}" | sed 's/^# \?//'
            exit 0
            ;;
        *) PASSTHROUGH+=("$1"); shift ;;
    esac
done

if [ ! -f "${WORLDGEN_ROOT}/deps/fastnoise2/CMakeLists.txt" ] \
    || [ ! -f "${WORLDGEN_ROOT}/deps/imglib/CMakeLists.txt" ]; then
    echo "fetching submodules"
    git -C "${WORLDGEN_ROOT}" submodule update --init --recursive
fi

# Where the binary for this configuration lands. A container build and a host build are kept apart,
# so which one to look for depends on how this is being run.
if [ "${NATIVE}" = "1" ]; then
    BUILD_PLATFORM_NAME="$(uname -s | tr '[:upper:]' '[:lower:]')"

    case "${BUILD_PLATFORM_NAME}" in
        linux)  BUILD_PLATFORM_NAME="linux" ;;
        darwin) BUILD_PLATFORM_NAME="macos" ;;
    esac
else
    BUILD_PLATFORM_NAME="linux-container"
fi

case "$(uname -m)" in
    x86_64|amd64)  BUILD_ARCH_NAME="x64" ;;
    aarch64|arm64) BUILD_ARCH_NAME="arm64" ;;
    *)             BUILD_ARCH_NAME="$(uname -m)" ;;
esac

BINARY="${WORLDGEN_ROOT}/build/${BUILD_ARCH_NAME}/${BUILD_PLATFORM_NAME}/${BUILD_TYPE}/bin/mapgen"

if [ "${NATIVE}" = "1" ]; then
    if [ "${REBUILD}" = "1" ] || [ ! -x "${BINARY}" ]; then
        echo "building ${BUILD_TYPE} with the host toolchain"
        "${WORLDGEN_ROOT}/scripts/build.sh" --type "${BUILD_TYPE}"
    else
        echo "using the existing ${BUILD_TYPE} build"
    fi

    exec "${WORLDGEN_ROOT}/scripts/run.sh" --type "${BUILD_TYPE}" \
        ${PASSTHROUGH+"${PASSTHROUGH[@]}"}
fi

if ! command -v docker >/dev/null 2>&1; then
    cat >&2 <<'EOF'
error: docker is not installed or not on PATH.

Either install docker, or build with the host's own toolchain instead:

    ./run.sh --native        (needs a compiler, cmake and VCPKG_ROOT set)
EOF
    exit 1
fi

DOCKER_FLAGS=()

if [ "${REBUILD_IMAGE}" = "1" ]; then
    DOCKER_FLAGS+=(--rebuild-image)
fi

# scripts/docker.sh builds the image itself when it is missing, so a first run needs no separate
# step - this only reports what is about to happen.
if [ "${REBUILD_IMAGE}" = "1" ] || ! docker image inspect "${WORLDGEN_IMAGE:-worldgen-build}" >/dev/null 2>&1; then
    echo "the build image is not present yet; it will be built first (a few minutes, once)"
fi

if [ "${REBUILD}" = "1" ] || [ ! -x "${BINARY}" ]; then
    echo "building ${BUILD_TYPE} in the container"
    "${WORLDGEN_ROOT}/scripts/docker.sh" ${DOCKER_FLAGS+"${DOCKER_FLAGS[@]}"} \
        ./scripts/build.sh --type "${BUILD_TYPE}"
else
    echo "using the existing ${BUILD_TYPE} build"
fi

if [ -z "${DISPLAY:-}" ] && [ -z "${WAYLAND_DISPLAY:-}" ]; then
    echo "error: no display. mapgen opens a window and needs DISPLAY set on the host." >&2
    exit 1
fi

echo "starting mapgen"
exec "${WORLDGEN_ROOT}/scripts/docker.sh" --gui \
    ./scripts/run.sh --type "${BUILD_TYPE}" ${PASSTHROUGH+"${PASSTHROUGH[@]}"}
