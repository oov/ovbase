#!/usr/bin/env bash

set -eu

function download_busybox () {
  url="$1"
  destdir="$2"
  if [ -d "${destdir}" ]; then
    # already installed
    return
  fi
  # download if not exists
  filename="$(basename "$url")"
  if [ ! -f "${filename}" ]; then
    echo "Downloading: ${url}"
    curl -sOL "$url"
  fi
  # install
  echo "Extracting: ${filename}"
  mkdir "${destdir}"
  ext="${filename##*.}"
  if [ "${ext}" = "exe" ]; then
    cp "${filename}" "${destdir}/busybox.exe"
  else
    cp "${filename}" "${destdir}/busybox"
    chmod +x "${destdir}/busybox"
  fi
  return
}

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
    echo "Downloading: ${url}"
    curl -sOL "$url"
  fi
  # install
  echo "Extracting: ${filename}"
  ext="${filename##*.}"
  if [ "${ext}" = "zip" ]; then
    busybox unzip -q "${filename}"
    noext="${filename%.*}"
    mv "${noext}" "${destdir}"
  elif [ "${ext}" = "gz" ]; then
    noext="${filename%.*}"
    noext="${noext%.tar}"
    # it seems that busybox tar is not fast on WSL so use cmake tar
    busybox tar xzf "${filename}" --exclude "${noext}/share" --exclude "${noext}/man" --exclude "${noext}/doc"
    "${noext}/bin/cmake" -E tar xf "${filename}"
    mv "${noext}" "${destdir}"
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
    echo "Downloading: ${url}"
    curl -sOL "$url"
  fi
  # install
  echo "Extracting: ${filename}"
  mkdir "${destdir}"
  cd "${destdir}"
  busybox unzip -q "../${filename}"
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
    echo "Downloading: ${url}"
    curl -sOL "$url"
  fi
  # install
  echo "Extracting: ${filename}"
  cmake -E tar xf "${filename}"
  noext="${filename%.*}"
  noext="${noext%.tar}"
  mv "${noext}" "${destdir}"
  return
}

function download_gettext () {
  url="$1"
  destdir="$2"
  if [ -d "${destdir}" ]; then
    # already installed
    return
  fi
  # download if not exists
  filename="$(basename "$url")"
  if [ ! -f "${filename}" ]; then
    echo "Downloading: ${url}"
    curl -sOL "$url"
  fi
  # install
  echo "Extracting: ${filename}"

  mkdir "${destdir}"
  cd "${destdir}"
  cmake -E tar xf "../${filename}"
  cd ..
  return
}

BUSYBOX_VERSION="dfe76d6"
CMAKE_VERSION="3.28.1"
NINJA_VERSION="1.11.1"
LLVM_MINGW_VERSION="20231128"
GETTEXT_VERSION="3fd6476"

case "$(uname -s)" in
  Linux*)
    platform="linux"
    BUSYBOX_URL="https://github.com/oov/busybox/releases/download/build-${BUSYBOX_VERSION}/busybox-linux-x86_64-${BUSYBOX_VERSION}"
    CMAKE_URL="https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-x86_64.tar.gz"
    NINJA_URL="https://github.com/ninja-build/ninja/releases/download/v${NINJA_VERSION}/ninja-linux.zip"
    LLVM_MINGW_URL="https://github.com/mstorsjo/llvm-mingw/releases/download/${LLVM_MINGW_VERSION}/llvm-mingw-${LLVM_MINGW_VERSION}-ucrt-ubuntu-20.04-x86_64.tar.xz"
    GETTEXT_URL="https://github.com/oov/gettext/releases/download/build-${GETTEXT_VERSION}/gettext-linux-x86_64-${GETTEXT_VERSION}.tar.xz"
    ;;
  MINGW64_NT* | MINGW32_NT*)
    platform="windows"
    BUSYBOX_URL="https://github.com/oov/busybox/releases/download/build-${BUSYBOX_VERSION}/busybox-windows-i686-${BUSYBOX_VERSION}.exe"
    CMAKE_URL="https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-windows-x86_64.zip"
    NINJA_URL="https://github.com/ninja-build/ninja/releases/download/v${NINJA_VERSION}/ninja-win.zip"
    LLVM_MINGW_URL="https://github.com/mstorsjo/llvm-mingw/releases/download/${LLVM_MINGW_VERSION}/llvm-mingw-${LLVM_MINGW_VERSION}-ucrt-x86_64.zip"
    GETTEXT_URL="https://github.com/oov/gettext/releases/download/build-${GETTEXT_VERSION}/gettext-windows-i686-${GETTEXT_VERSION}.zip"
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

OLD_PATH="$PATH"
export PATH="$PWD/cmake-${platform}/bin:$PWD/busybox-${platform}:$PATH"
download_busybox "${BUSYBOX_URL}" "${PWD}/busybox-${platform}"
download_cmake "${CMAKE_URL}" "${PWD}/cmake-${platform}"
download_ninja "${NINJA_URL}" "${PWD}/ninja-${platform}"
download_llvm_mingw "${LLVM_MINGW_URL}" "${PWD}/llvm-mingw-${platform}"
download_gettext "${GETTEXT_URL}" "${PWD}/gettext-${platform}"
export PATH="$OLD_PATH"

echo "export PATH=\"$PWD/busybox-${platform}:\$PATH\"" > env.sh
echo "export PATH=\"$PWD/cmake-${platform}/bin:\$PATH\"" >> env.sh
echo "export PATH=\"$PWD/ninja-${platform}:\$PATH\"" >> env.sh
echo "export PATH=\"$PWD/llvm-mingw-${platform}/bin:\$PATH\"" >> env.sh
echo "export PATH=\"$PWD/gettext-${platform}/bin:\$PATH\"" >> env.sh
chmod +x env.sh
. env.sh

cd "${CUR_DIR}"
