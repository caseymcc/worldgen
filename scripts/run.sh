#!/usr/bin/env bash
# Run the viewer out of a build tree.
#
#   ./scripts/run.sh              the default release build
#   ./scripts/run.sh --debug
#
# mapgen writes imgui.ini beside itself, so it runs from its own directory rather than the source
# tree. It generates the whole world before the window becomes responsive; that pause is the
# overview pipeline, not a hang.

set -euo pipefail

source "$(dirname "${BASH_SOURCE[0]}")/env.sh"

worldgen_parse_args "$@"
worldgen_resolve

BINARY="${BUILD_DIR}/bin/mapgen"

if [ ! -x "${BINARY}" ]; then
    echo "error: ${BINARY} does not exist. Build it with: ./scripts/build.sh -t ${BUILD_TYPE}" >&2
    exit 1
fi

if [ -z "${DISPLAY:-}" ] && [ -z "${WAYLAND_DISPLAY:-}" ]; then
    echo "error: no display. mapgen opens a window and needs DISPLAY or WAYLAND_DISPLAY set." >&2
    exit 1
fi

cd "${BUILD_DIR}/bin"
exec "./mapgen" ${WORLDGEN_ARGS+"${WORLDGEN_ARGS[@]}"}
