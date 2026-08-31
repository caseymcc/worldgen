#!/usr/bin/env bash
# Run a command inside the build container. The host needs docker and nothing else.
#
#   ./scripts/docker.sh                          build the default configuration
#   ./scripts/docker.sh ./scripts/build.sh --debug
#   ./scripts/docker.sh ./scripts/configure.sh --clean
#   ./scripts/docker.sh bash                     a shell in the build environment
#   ./scripts/docker.sh --gui ./scripts/run.sh   run the viewer on the host's display
#
#   --rebuild-image   build the image again even if it already exists
#   --gui             pass the host display through so a window can open
#
# The repository is bind mounted rather than copied, so edits on the host are what gets built, and
# the container runs as the calling user so nothing it writes ends up owned by root.

set -euo pipefail

WORLDGEN_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

IMAGE="${WORLDGEN_IMAGE:-worldgen-build}"
CACHE_VOLUME="${WORLDGEN_CACHE_VOLUME:-worldgen-vcpkg-cache}"
REBUILD=0
GUI=0

while [ $# -gt 0 ]; do
    case "$1" in
        --rebuild-image) REBUILD=1; shift ;;
        --gui)           GUI=1; shift ;;
        -h|--help)
            sed -n '2,17p' "${BASH_SOURCE[0]}" | sed 's/^# \?//'
            exit 0
            ;;
        *) break ;;
    esac
done

if [ $# -eq 0 ]; then
    set -- ./scripts/build.sh
fi

if ! command -v docker >/dev/null 2>&1; then
    echo "error: docker is not installed or not on PATH." >&2
    exit 1
fi

if [ "${REBUILD}" = "1" ] || ! docker image inspect "${IMAGE}" >/dev/null 2>&1; then
    echo "building image ${IMAGE} (first time takes a few minutes)"
    docker build -t "${IMAGE}" -f "${WORLDGEN_ROOT}/docker/Dockerfile" "${WORLDGEN_ROOT}/docker"
fi

# Ports are compiled once and kept in a named volume, so a second run neither downloads nor rebuilds
# them. Removing it (docker volume rm worldgen-vcpkg-cache) forces that work to happen again.
docker volume inspect "${CACHE_VOLUME}" >/dev/null 2>&1 || docker volume create "${CACHE_VOLUME}" >/dev/null

RUN_ARGS=(
    --rm
    --user "$(id -u):$(id -g)"
    --volume "${WORLDGEN_ROOT}:/work"
    --volume "${CACHE_VOLUME}:/cache"
    --workdir /work
    --env HOME=/tmp
)

if [ -t 0 ] && [ -t 1 ]; then
    RUN_ARGS+=(--interactive --tty)
fi

if [ "${GUI}" = "1" ]; then
    if [ -n "${DISPLAY:-}" ]; then
        RUN_ARGS+=(--env "DISPLAY=${DISPLAY}" --network host)

        if [ -d /tmp/.X11-unix ]; then
            RUN_ARGS+=(--volume /tmp/.X11-unix:/tmp/.X11-unix:ro)
        fi

        if [ -n "${XAUTHORITY:-}" ] && [ -f "${XAUTHORITY}" ]; then
            RUN_ARGS+=(--volume "${XAUTHORITY}:/tmp/.Xauthority:ro" --env XAUTHORITY=/tmp/.Xauthority)
        fi
    else
        echo "warning: --gui given but DISPLAY is not set on the host" >&2
    fi

    # a GPU if there is one to hand through, otherwise mesa in the image renders in software
    if [ -d /dev/dri ]; then
        RUN_ARGS+=(--device /dev/dri)
    fi
fi

exec docker run "${RUN_ARGS[@]}" "${IMAGE}" "$@"
