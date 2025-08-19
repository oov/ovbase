#!/bin/sh

set -eu

get_extension() {
  filename=$(basename "$1")
  case $filename in
    *.tar.*)
      echo "$filename" | sed 's/.*\(tar\.[^.]*\)$/\1/' ;;
    *.*)
      echo "$filename" | sed 's/.*\.//' ;;
    *)
      echo "" ;;
  esac
}

remove_extension() {
  filename="$1"
  ext="$2"
  extlen=$(($(echo "$ext" | wc -c)))
  echo "$filename" | sed "s/.\{$extlen\}$//"
}

create_temp_dir() {
  prefix="setup_tmp"
  datetime=$(date +%Y%m%d%H%M%S)
  pid="$$"
  random=""
  if [ -r "/dev/urandom" ]; then
    random=$(head -c6 /dev/urandom | od -t u1 | sed '1!d;s/^.\{8\}//;s/[[:space:]]//g' | head -c6)
  else
    random=$(date +%N 2>/dev/null | head -c6 || echo "000000")
  fi
  temp_dir="${prefix}_${datetime}_${random}_${pid}"
  mkdir -p "${temp_dir}"
  echo "${temp_dir}"
}

download_busybox () {
  url="$1"
  version="$2"
  filename=$(basename "${url}")
  ext=$(get_extension "${filename}")
  if [ -z "${ext}" ]; then
    filename="${filename}.bin"
    ext="bin"
  fi
  destdir="${PWD}/$(remove_extension "${filename}" "${ext}")"
  if [ -d "${destdir}" ]; then
    # already installed
    echo "${destdir}"
    return
  fi
  # download if not exists
  filepath="${PWD}/${filename}"
  if [ ! -f "${filepath}" ]; then
    echo "Downloading: ${url}" >&2
    curl -sL "$url" -o "${filepath}"
  fi
  # install
  echo "Extracting: ${filepath}" >&2
  mkdir "${destdir}"
  if [ "${ext}" = "exe" ]; then
    cp "${filepath}" "${destdir}/busybox.exe"
  else
    cp "${filepath}" "${destdir}/busybox"
    chmod +x "${destdir}/busybox"
  fi
  echo "${destdir}"
  return
}

download_cmake () {
  url="$1"
  version="$2"
  filename=$(basename "${url}")
  ext=$(get_extension "${filename}")
  destdir="${PWD}/$(remove_extension "${filename}" "${ext}")"
  if [ -d "${destdir}" ]; then
    # already installed
    echo "${destdir}"
    return
  fi
  # download if not exists
  filepath="${PWD}/${filename}"
  if [ ! -f "${filepath}" ]; then
    echo "Downloading: ${url}" >&2
    curl -sL "$url" -o "${filepath}"
  fi
  # install
  echo "Extracting: ${filepath}" >&2
  mkdir -p "${destdir}"
  temp_dir=$(create_temp_dir)
  if [ "${ext}" = "zip" ]; then
    (cd "${temp_dir}" && busybox unzip -q "${filepath}")
  else
    # it seems that busybox tar is not fast on WSL so use cmake tar
    noext=$(remove_extension "${filename}" "${ext}")
    (cd "${temp_dir}" && busybox tar xzf "${filepath}" --exclude "${noext}/share" --exclude "${noext}/man" --exclude "${noext}/doc")
    (cd "${temp_dir}" && mv "${noext}/bin/cmake" ./ && ./cmake -E tar xf "${filepath}" && rm -rf cmake)
  fi
  mv "${temp_dir}"/*/* "${destdir}/"
  rm -rf "${temp_dir}"
  echo "${destdir}"
  return
}

download_ninja () {
  url="$1"
  version="$2"
  filename=$(basename "${url}")
  ext=$(get_extension "${filename}")
  filename="${filename%.$ext}-${version}.${ext}"
  destdir="${PWD}/$(remove_extension "${filename}" "${ext}")"
  if [ -d "${destdir}" ]; then
    # already installed
    echo "${destdir}"
    return
  fi
  filepath="${PWD}/${filename}"
  if [ ! -f "${filepath}" ]; then
    echo "Downloading: ${url}" >&2
    curl -sL "$url" -o "${filepath}"
  fi
  # install
  echo "Extracting: ${filepath}" >&2
  mkdir "${destdir}"
  (cd "${destdir}" && busybox unzip -q "${filepath}")
  echo "${destdir}"
  return
}

download_llvm_mingw () {
  url="$1"
  version="$2"
  filename=$(basename "${url}")
  ext=$(get_extension "${filename}")
  destdir="${PWD}/$(remove_extension "${filename}" "${ext}")"
  if [ -d "${destdir}" ]; then
    # already installed
    echo "${destdir}"
    return
  fi
  # download if not exists
  filepath="${PWD}/${filename}"
  if [ ! -f "${filepath}" ]; then
    echo "Downloading: ${url}" >&2
    curl -sL "$url" -o "${filepath}"
  fi
  # install
  echo "Extracting: ${filepath}" >&2
  mkdir -p "${destdir}"
  temp_dir=$(create_temp_dir)
  if [ "${ext}" = "zip" ]; then
    (cd "${temp_dir}" && busybox unzip -q "${filepath}")
  else
    (cd "${temp_dir}" && cmake -E tar xf "${filepath}")
  fi
  mv "${temp_dir}"/*/* "${destdir}/" 2>/dev/null
  rm -rf "${temp_dir}"
  echo "${destdir}"
  return
}

download_gettext () {
  url="$1"
  version="$2"
  filename=$(basename "${url}")
  ext=$(get_extension "${filename}")
  destdir="${PWD}/$(remove_extension "${filename}" "${ext}")"
  if [ -d "${destdir}" ]; then
    # already installed
    echo "${destdir}"
    return
  fi
  # download if not exists
  filepath="${PWD}/${filename}"
  if [ ! -f "${filepath}" ]; then
    echo "Downloading: ${url}" >&2
    curl -sL "$url" -o "${filepath}"
  fi
  # install
  echo "Extracting: ${filepath}" >&2
  mkdir "${destdir}"
  (cd "${destdir}" && cmake -E tar xf "${filepath}")
  echo "${destdir}"
  return
}

BUSYBOX_VERSION="dfe76d6"
CMAKE_VERSION="4.1.0"
NINJA_VERSION="1.13.1"
LLVM_MINGW_VERSION="20250709"
GETTEXT_VERSION="7c5c644"

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

while [ $# -gt 0 ]; do
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
busybox_dir=$(download_busybox "${BUSYBOX_URL}" "${BUSYBOX_VERSION}")
export PATH="${busybox_dir}:$PATH"
cmake_dir=$(download_cmake "${CMAKE_URL}" "${CMAKE_VERSION}")
export PATH="${cmake_dir}/bin:$PATH"

ninja_dir=$(download_ninja "${NINJA_URL}" "${NINJA_VERSION}")
llvm_mingw_dir=$(download_llvm_mingw "${LLVM_MINGW_URL}" "${LLVM_MINGW_VERSION}")
gettext_dir=$(download_gettext "${GETTEXT_URL}" "${GETTEXT_VERSION}")
export PATH="$OLD_PATH"

envname="env-${platform}.sh"
echo "export PATH=\"${busybox_dir}:\$PATH\"" > "${envname}"
echo "export PATH=\"${cmake_dir}/bin:\$PATH\"" >> "${envname}"
echo "export PATH=\"${ninja_dir}:\$PATH\"" >> "${envname}"
echo "export PATH=\"${llvm_mingw_dir}/bin:\$PATH\"" >> "${envname}"
echo "export PATH=\"${gettext_dir}/bin:\$PATH\"" >> "${envname}"
chmod +x "${envname}"
. "${envname}"

cd "${CUR_DIR}"
