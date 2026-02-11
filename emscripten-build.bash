#!/bin/bash
#
# Emscripten build & test script for ovbase.
#
# Usage:
#   bash emscripten-build.bash
#
# Outside Docker: builds a cached Docker image and re-invokes itself inside.
# Inside Docker (/.dockerenv): runs the actual build.

set -eu

# ---------------------------------------------------------------------------
# Host-side: wrap ourselves in a cached Docker container.
# ---------------------------------------------------------------------------
if [ ! -f /.dockerenv ]; then
  IMAGE=ovbase-emscripten-build
  EMSDK_VERSION=3.1.73
  if ! docker image inspect "${IMAGE}:${EMSDK_VERSION}" >/dev/null 2>&1; then
    echo "=== Building Docker image '${IMAGE}:${EMSDK_VERSION}' ==="
    docker build -t "${IMAGE}:${EMSDK_VERSION}" --build-arg EMSDK_VERSION="${EMSDK_VERSION}" - <<'DOCKERFILE'
ARG EMSDK_VERSION=3.1.73
FROM emscripten/emsdk:${EMSDK_VERSION}
RUN apt-get update && apt-get install -y --no-install-recommends \
    ninja-build \
    && rm -rf /var/lib/apt/lists/*
DOCKERFILE
  fi
  SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
  exec docker run --rm \
    -v "${SCRIPT_DIR}:/src/ovbase" \
    -w /src/ovbase \
    "${IMAGE}:${EMSDK_VERSION}" \
    bash ./emscripten-build.bash
fi

# ---------------------------------------------------------------------------
# Container-side: perform the actual Emscripten build.
# ---------------------------------------------------------------------------

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build-emscripten"

mkdir -p "${BUILD_DIR}"

echo "=== Building ovbase with Emscripten ==="
emcmake cmake \
  -S "${SCRIPT_DIR}" \
  -B "${BUILD_DIR}" \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DTARGET_EMSCRIPTEN=ON \
  -DFORMAT_SOURCES=OFF \
  -DUSE_LTO=OFF \
  -DLDNAME="" \
  -DSKIP_OVTHREADS_TEST=ON \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build "${BUILD_DIR}"

echo "=== Running tests ==="
ctest --test-dir "${BUILD_DIR}" --output-on-failure

echo "=== Build complete ==="
