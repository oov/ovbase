#!/bin/bash
#
# Emscripten build & test script for ovbase.
#
# Usage:
#   bash emscripten-build.bash        # Release build
#   bash emscripten-build.bash -d     # Debug build
#   bash emscripten-build.bash -s     # Skip tests
#   bash emscripten-build.bash -r     # Force rebuild
#
# Outside Docker: builds a cached Docker image and re-invokes itself inside.
# Inside Docker (/.dockerenv): runs the actual build.

set -eu

CMAKE_BUILD_TYPE=Release
REBUILD=0
SKIP_TESTS=0
while [[ $# -gt 0 ]]; do
  case $1 in
    -d|--debug)
      CMAKE_BUILD_TYPE=Debug
      shift
      ;;
    -r|--rebuild)
      REBUILD=1
      shift
      ;;
    -s|--skip-tests)
      SKIP_TESTS=1
      shift
      ;;
    *)
      shift
      ;;
  esac
done

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
  ARGS=()
  [ "${CMAKE_BUILD_TYPE}" = "Debug" ] && ARGS+=(-d)
  [ "${REBUILD}" -eq 1 ] && ARGS+=(-r)
  [ "${SKIP_TESTS}" -eq 1 ] && ARGS+=(-s)
  exec docker run --rm \
    -v "${SCRIPT_DIR}:/src/ovbase" \
    -w /src/ovbase \
    "${IMAGE}:${EMSDK_VERSION}" \
    bash ./emscripten-build.bash "${ARGS[@]+"${ARGS[@]}"}"
fi

# ---------------------------------------------------------------------------
# Container-side: perform the actual Emscripten build.
# ---------------------------------------------------------------------------

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build/${CMAKE_BUILD_TYPE}/emscripten"

if [ "${REBUILD}" -eq 1 ] && [ -d "${BUILD_DIR}" ]; then
  rm -rf "${BUILD_DIR}"
fi
mkdir -p "${BUILD_DIR}"

echo "=== Building ovbase with Emscripten (${CMAKE_BUILD_TYPE}) ==="
emcmake cmake \
  -S "${SCRIPT_DIR}" \
  -B "${BUILD_DIR}" \
  -G Ninja \
  -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
  -DTARGET_EMSCRIPTEN=ON \
  -DFORMAT_SOURCES=OFF \
  -DUSE_LTO=OFF \
  -DLDNAME="" \
  -DSKIP_OVTHREADS_TEST=ON \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build "${BUILD_DIR}"

if [ "${SKIP_TESTS}" -eq 0 ]; then
  echo "=== Running tests ==="
  ctest --test-dir "${BUILD_DIR}" --output-on-failure
fi

echo "=== Build complete ==="
