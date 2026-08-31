#!/usr/bin/env bash
# Build the default configuration, configuring it first if that has not been done.
#
#   ./scripts/build.sh                   release, host architecture, everything
#   ./scripts/build.sh --debug
#   ./scripts/build.sh --target worldgen
#   ./scripts/build.sh -j 4
#
# Output lands in build/<arch>/<platform>/<type>, binaries in that tree's bin/.

set -euo pipefail

source "$(dirname "${BASH_SOURCE[0]}")/env.sh"

TARGET=""
JOBS=""
CLEAN=0
EXTRA=()

worldgen_parse_args "$@"

set -- ${WORLDGEN_ARGS+"${WORLDGEN_ARGS[@]}"}

while [ $# -gt 0 ]; do
    case "$1" in
        --target)  TARGET="$2"; shift 2 ;;
        -j|--jobs) JOBS="$2"; shift 2 ;;
        --clean)   CLEAN=1; shift ;;
        -h|--help)
            sed -n '2,9p' "${BASH_SOURCE[0]}" | sed 's/^# \?//'
            exit 0
            ;;
        *) EXTRA+=("$1"); shift ;;
    esac
done

worldgen_resolve

if [ -z "${JOBS}" ]; then
    JOBS="$( (nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4) )"
fi

if [ ! -f "${BUILD_DIR}/CMakeCache.txt" ]; then
    echo "no configured tree at ${BUILD_DIR}, configuring first"
    "${WORLDGEN_ROOT}/scripts/configure.sh" \
        --type "${BUILD_TYPE}" --arch "${BUILD_ARCH}" --platform "${BUILD_PLATFORM}" \
        --generator "${BUILD_GENERATOR}"
fi

if [ "${CLEAN}" = "1" ]; then
    cmake --build "${BUILD_DIR}" --target clean
fi

echo "building ${BUILD_ARCH}/${BUILD_PLATFORM}/${BUILD_TYPE} with ${JOBS} jobs"

if [ -n "${TARGET}" ]; then
    cmake --build "${BUILD_DIR}" --target "${TARGET}" -j "${JOBS}" ${EXTRA+"${EXTRA[@]}"}
else
    cmake --build "${BUILD_DIR}" -j "${JOBS}" ${EXTRA+"${EXTRA[@]}"}
fi

echo
echo "built into ${BUILD_DIR}"

if [ -x "${BUILD_DIR}/bin/mapgen" ]; then
    echo "run the viewer with: ./scripts/run.sh -t ${BUILD_TYPE}"
fi
