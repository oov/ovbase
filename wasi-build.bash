#!/bin/bash

set -eu

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ARCHDIR=${ARCHDIR:-wasm32}
BUILD_DIR="${SCRIPT_DIR}/build/Debug/${ARCHDIR}"

# Configuration
EMSDK_VERSION=3.1.44
WASMTIME_VERSION=v36.0.2

# Save original directory
ORIGINAL_DIR="$(pwd)"

# Create build directory
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Install wasmtime if not present
if [ ! -d "wasmtime-${WASMTIME_VERSION}-x86_64-linux" ]; then
    echo "Installing wasmtime ${WASMTIME_VERSION}..."
    wget "https://github.com/bytecodealliance/wasmtime/releases/download/${WASMTIME_VERSION}/wasmtime-${WASMTIME_VERSION}-x86_64-linux.tar.xz"
    tar Jxf "wasmtime-${WASMTIME_VERSION}-x86_64-linux.tar.xz"
fi

# Install WASI SDK if not present
if [ ! -d "wasi-sdk-27.0-x86_64-linux" ]; then
    echo "Installing WASI SDK..."
    wget "https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-27/wasi-sdk-27.0-x86_64-linux.tar.gz"
    tar zxf "wasi-sdk-27.0-x86_64-linux.tar.gz"
fi

# Add wasmtime to PATH
export PATH="$(pwd)/wasmtime-${WASMTIME_VERSION}-x86_64-linux:$PATH"
export WASISDK="$(pwd)/wasi-sdk-27.0-x86_64-linux"

# Return to original directory
cd "${ORIGINAL_DIR}"

# Build with Emscripten
CC="${WASISDK}/bin/clang-20 --target=wasm32-wasi-threads --sysroot=${WASISDK}/share/wasi-sysroot -pthread" \
  cmake \
    -S "${SCRIPT_DIR}" \
    -B "${BUILD_DIR}" \
    --preset debug \
    -DTARGET_WASI_SDK=ON \
    -DFORMAT_SOURCES=OFF \
    -DSKIP_OVTHREADS_TEST=OFF \
    -DUSE_LTO=OFF \
    -DLDNAME="" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build "${BUILD_DIR}"
ctest --test-dir "${BUILD_DIR}" --output-on-failure --output-junit testlog.xml
