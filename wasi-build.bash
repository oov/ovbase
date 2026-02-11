#!/bin/bash
#
# WASI build & test script for ovbase.
#
# Usage:
#   bash wasi-build.bash        # Debug build (default)
#   bash wasi-build.bash -s     # Skip tests
#   bash wasi-build.bash -r     # Force rebuild
#
# Outside Docker: builds a cached Docker image and re-invokes itself inside.
# Inside Docker (/.dockerenv): runs the actual build.

set -eu

CMAKE_BUILD_TYPE=Debug
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
  IMAGE=ovbase-wasi-build
  WASI_SDK_VERSION=27
  WASMTIME_VERSION=v36.0.2
  if ! docker image inspect "${IMAGE}:wasi${WASI_SDK_VERSION}" >/dev/null 2>&1; then
    echo "=== Building Docker image '${IMAGE}:wasi${WASI_SDK_VERSION}' ==="
    docker build -t "${IMAGE}:wasi${WASI_SDK_VERSION}" \
      --build-arg WASI_SDK_VERSION="${WASI_SDK_VERSION}" \
      --build-arg WASMTIME_VERSION="${WASMTIME_VERSION}" \
      - <<'DOCKERFILE'
FROM ubuntu:jammy
ARG WASI_SDK_VERSION=27
ARG WASMTIME_VERSION=v36.0.2
RUN apt-get update && apt-get install -y --no-install-recommends \
    cmake ninja-build wget ca-certificates xz-utils \
    && rm -rf /var/lib/apt/lists/*
RUN wget -q "https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-${WASI_SDK_VERSION}/wasi-sdk-${WASI_SDK_VERSION}.0-x86_64-linux.tar.gz" \
    && tar zxf "wasi-sdk-${WASI_SDK_VERSION}.0-x86_64-linux.tar.gz" -C /opt \
    && rm "wasi-sdk-${WASI_SDK_VERSION}.0-x86_64-linux.tar.gz"
RUN wget -q "https://github.com/bytecodealliance/wasmtime/releases/download/${WASMTIME_VERSION}/wasmtime-${WASMTIME_VERSION}-x86_64-linux.tar.xz" \
    && tar Jxf "wasmtime-${WASMTIME_VERSION}-x86_64-linux.tar.xz" -C /opt \
    && rm "wasmtime-${WASMTIME_VERSION}-x86_64-linux.tar.xz"
ENV WASISDK=/opt/wasi-sdk-${WASI_SDK_VERSION}.0-x86_64-linux
ENV PATH="/opt/wasmtime-${WASMTIME_VERSION}-x86_64-linux:${PATH}"
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
    "${IMAGE}:wasi${WASI_SDK_VERSION}" \
    bash ./wasi-build.bash "${ARGS[@]+"${ARGS[@]}"}"
fi

# ---------------------------------------------------------------------------
# Container-side: perform the actual WASI build.
# ---------------------------------------------------------------------------

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build/${CMAKE_BUILD_TYPE}/wasm32"

if [ "${REBUILD}" -eq 1 ] && [ -d "${BUILD_DIR}" ]; then
  rm -rf "${BUILD_DIR}"
fi
mkdir -p "${BUILD_DIR}"

echo "=== Building ovbase with WASI SDK (${CMAKE_BUILD_TYPE}) ==="
CC="${WASISDK}/bin/clang-20 --target=wasm32-wasi-threads --sysroot=${WASISDK}/share/wasi-sysroot -pthread" \
  cmake \
    -S "${SCRIPT_DIR}" \
    -B "${BUILD_DIR}" \
    -G Ninja \
    -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
    -DTARGET_WASI_SDK=ON \
    -DFORMAT_SOURCES=OFF \
    -DSKIP_OVTHREADS_TEST=OFF \
    -DUSE_LTO=OFF \
    -DLDNAME="" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build "${BUILD_DIR}"

if [ "${SKIP_TESTS}" -eq 0 ]; then
  echo "=== Running tests ==="
  ctest --test-dir "${BUILD_DIR}" --output-on-failure
fi

echo "=== Build complete ==="
