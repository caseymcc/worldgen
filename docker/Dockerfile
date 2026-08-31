# Build environment for worldgen. Everything the project needs to compile lives in here, so a host
# only needs docker: no compiler, no cmake, no vcpkg, no X11 development packages.
#
# The image deliberately does NOT contain the source. The repository is bind mounted at build time,
# so editing a file does not invalidate the image and the toolchain is built once.
FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive

# Pinned so the image is reproducible. Override with --build-arg VCPKG_COMMIT=<sha> to move it.
ARG VCPKG_COMMIT=e3ed41868d5034bc608eaaa58383cd6ecdbb5ffb

RUN apt-get update && apt-get install --no-install-recommends -y \
        build-essential \
        cmake \
        ninja-build \
        git \
        curl \
        zip \
        unzip \
        tar \
        pkg-config \
        ca-certificates \
        python3 \
        nasm \
        autoconf \
        automake \
        libtool \
    && rm -rf /var/lib/apt/lists/*

# glfw3, glew and glbinding compile against the X11 and OpenGL headers; the wayland and xkb packages
# are here because recent glfw configures for both display backends and fails without them.
RUN apt-get update && apt-get install --no-install-recommends -y \
        libx11-dev \
        libxrandr-dev \
        libxinerama-dev \
        libxcursor-dev \
        libxi-dev \
        libxext-dev \
        libxkbcommon-dev \
        libwayland-dev \
        wayland-protocols \
        libgl1-mesa-dev \
        libglu1-mesa-dev \
        libegl1-mesa-dev \
    && rm -rf /var/lib/apt/lists/*

# Mesa's software renderer, so the viewer can run from the container against a forwarded display
# even where there is no GPU to talk to.
RUN apt-get update && apt-get install --no-install-recommends -y \
        libgl1-mesa-dri \
        libglx-mesa0 \
    && rm -rf /var/lib/apt/lists/*

ENV VCPKG_ROOT=/opt/vcpkg
ENV VCPKG_DISABLE_METRICS=1

RUN git clone https://github.com/microsoft/vcpkg.git ${VCPKG_ROOT} \
    && cd ${VCPKG_ROOT} \
    && if [ "${VCPKG_COMMIT}" != "latest" ]; then git checkout --quiet "${VCPKG_COMMIT}" || \
         echo "warning: VCPKG_COMMIT ${VCPKG_COMMIT} not found, staying on the default branch"; fi \
    && ./bootstrap-vcpkg.sh -disableMetrics \
    && rm -rf ${VCPKG_ROOT}/downloads ${VCPKG_ROOT}/buildtrees

# The container runs as the calling user so build output is owned by them and not by root. That user
# does not exist inside the image, so anything vcpkg writes under its own root has to be writable by
# any uid.
RUN chmod -R a+rwX ${VCPKG_ROOT}

# Ports are downloaded and cached under here, which the run script mounts as a volume so a second
# build does not fetch or compile them again.
ENV VCPKG_DOWNLOADS=/cache/downloads
ENV VCPKG_DEFAULT_BINARY_CACHE=/cache/binary
RUN mkdir -p /cache/downloads /cache/binary && chmod -R a+rwX /cache

# A container build and a host build are not interchangeable - different toolchain, different vcpkg
# tree - so they get separate directories and neither clobbers the other. scripts/env.sh honours
# this, and --platform still overrides it.
ENV BUILD_PLATFORM=linux-container

WORKDIR /work

CMD ["./scripts/build.sh"]
