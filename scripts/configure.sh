#!/usr/bin/env bash
# Configure a build tree with the project's default settings.
#
#   ./scripts/configure.sh                       release, host architecture
#   ./scripts/configure.sh --debug
#   ./scripts/configure.sh -t debug -DWORLDGEN_MAPGEN=OFF
#
# The tree goes to build/<arch>/<platform>/<type>, e.g. build/x64/linux/release.

set -euo pipefail

source "$(dirname "${BASH_SOURCE[0]}")/env.sh"

CLEAN=0
EXTRA=()

worldgen_parse_args "$@"

for arg in ${WORLDGEN_ARGS+"${WORLDGEN_ARGS[@]}"}; do
    case "${arg}" in
        --clean) CLEAN=1 ;;
        -h|--help)
            sed -n '2,9p' "${BASH_SOURCE[0]}" | sed 's/^# \?//'
            exit 0
            ;;
        *) EXTRA+=("${arg}") ;;
    esac
done

worldgen_resolve
worldgen_check_submodules
worldgen_check_vcpkg

if [ "${CLEAN}" = "1" ] && [ -d "${BUILD_DIR}" ]; then
    echo "removing ${BUILD_DIR}"
    rm -rf "${BUILD_DIR}"
fi

mkdir -p "${BUILD_DIR}"

echo "configuring ${BUILD_ARCH}/${BUILD_PLATFORM}/${BUILD_TYPE}"
echo "  source    ${WORLDGEN_ROOT}"
echo "  build     ${BUILD_DIR}"
echo "  generator ${BUILD_GENERATOR}"
echo "  vcpkg     ${VCPKG_ROOT}"

# The first configure is slow: vcpkg compiles every port in vcpkg.json, and with the mapgen feature
# on that includes glfw, glew, glbinding and imgui.
cmake \
    -S "${WORLDGEN_ROOT}" \
    -B "${BUILD_DIR}" \
    -G "${BUILD_GENERATOR}" \
    -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    ${EXTRA+"${EXTRA[@]}"}

# Editors and clang tooling look for this at the root rather than inside a build tree.
if [ -f "${BUILD_DIR}/compile_commands.json" ]; then
    ln -sf "${BUILD_DIR}/compile_commands.json" "${WORLDGEN_ROOT}/compile_commands.json"
fi

echo "configured. build it with: ./scripts/build.sh -t ${BUILD_TYPE}"
