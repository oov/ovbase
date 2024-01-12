#!/usr/bin/env bash

set -eu

function download_cmake () {
  url="$1"
  destdir="$2"
  if [ -d "${destdir}" ]; then
    # already installed
    return
  fi
  # download if not exists
  filename="$(basename "$url")"
  if [ ! -f "${filename}" ]; then
    curl -OL "$url"
  fi
  # install
  ext="${filename##*.}"
  if [ "${ext}" = "zip" ]; then
    unzip -q "${filename}"
    noext="${filename%.*}"
    mv "${noext}" "${destdir}"
  elif [ "${ext}" = "sh" ]; then
    mkdir "${destdir}"
    sh "${filename}" --skip-license --prefix="${destdir}"
  fi
  return
}

function download_ninja () {
  url="$1"
  destdir="$2"
  if [ -d "${destdir}" ]; then
    # already installed
    return
  fi
  # download if not exists
  filename="$(basename "$url")"
  if [ ! -f "${filename}" ]; then
    curl -OL "$url"
  fi
  # install
  mkdir "${destdir}"
  cd "${destdir}"
  cmake -E tar xf "../${filename}"
  cd ..
  return
}

function download_llvm_mingw () {
  url="$1"
  destdir="$2"
  if [ -d "${destdir}" ]; then
    # already installed
    return
  fi
  # download if not exists
  filename="$(basename "$url")"
  if [ ! -f "${filename}" ]; then
    curl -OL "$url"
  fi
  # install
  cmake -E tar xf "${filename}"
  noext="${filename%.*}"
  noext="${noext%.tar}"
  mv "${noext}" "${destdir}"
  return
}

CMAKE_VERSION="3.28.1"
NINJA_VERSION="1.11.1"
LLVM_MINGW_VERSION="20231128"

case "$(uname -s)" in
  Linux*)
    platform="linux"
    CMAKE_URL="https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-x86_64.sh"
    NINJA_URL="https://github.com/ninja-build/ninja/releases/download/v${NINJA_VERSION}/ninja-linux.zip"
    LLVM_MINGW_URL="https://github.com/mstorsjo/llvm-mingw/releases/download/${LLVM_MINGW_VERSION}/llvm-mingw-${LLVM_MINGW_VERSION}-ucrt-ubuntu-20.04-x86_64.tar.xz"
    ;;
  MINGW64_NT* | MINGW32_NT*)
    platform="windows"
    CMAKE_URL="https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-windows-x86_64.zip"
    NINJA_URL="https://github.com/ninja-build/ninja/releases/download/v${NINJA_VERSION}/ninja-win.zip"
    LLVM_MINGW_URL="https://github.com/mstorsjo/llvm-mingw/releases/download/${LLVM_MINGW_VERSION}/llvm-mingw-${LLVM_MINGW_VERSION}-ucrt-x86_64.zip"
    ;;
  *)
    echo "Unsupported platform: $(uname -s)"
    exit 1
    ;;
esac

CUR_DIR="${PWD}"
DEST_DIR=""

while [[ $# -gt 0 ]]; do
  case $1 in
    -d|--dir)
      DEST_DIR=$2
      shift
      shift
      ;;
    -*|--*)
      echo "Unknown option $1"
      exit 1
      ;;
    *)
      shift
      ;;
  esac
done

if [ -z "${DEST_DIR}" ]; then
  cd "$(dirname "${BASH_SOURCE:-$0}")"
  mkdir -p build/tools
  cd build/tools
else
  cd "${DEST_DIR}"
fi

download_cmake ${CMAKE_URL} "${PWD}/cmake-${platform}"
export PATH=$PWD/cmake-${platform}/bin:$PATH
download_ninja ${NINJA_URL} "${PWD}/ninja-${platform}"
download_llvm_mingw ${LLVM_MINGW_URL} "${PWD}/llvm-mingw-${platform}"

echo "export PATH=$PWD/cmake-${platform}/bin:\$PATH" > env.sh
echo "export PATH=$PWD/ninja-${platform}:\$PATH" >> env.sh
echo "export PATH=$PWD/llvm-mingw-${platform}/bin:\$PATH" >> env.sh
chmod +x env.sh
. env.sh

cd "${CUR_DIR}"
