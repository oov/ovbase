#!/usr/bin/env bash
set -eu

CUR_DIR="${PWD}"
cd "$(dirname "${BASH_SOURCE:-$0}")"

INSTALL_TOOLS=1
REBUILD=0
SKIP_TESTS=0
CMAKE_PRESET=release
FORMAT_SOURCES=ON
while [[ $# -gt 0 ]]; do
  case $1 in
    --no-install-tools)
      INSTALL_TOOLS=0
      shift
      ;;
    -d|--debug)
      CMAKE_PRESET=debug
      shift
      ;;
    --no-format)
      FORMAT_SOURCES=OFF
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
    -*|--*)
      echo "Unknown option $1"
      exit 1
      ;;
    *)
      shift
      ;;
  esac
done

# Run command with progress indicator (prints . every second)
# Usage: run_with_progress <message> <log_file> <command...>
run_with_progress() {
  local message="$1"
  local log_file="$2"
  shift 2
  printf "%s" "${message}"
  local start_time=$(date +%s.%N)
  "$@" > "${log_file}" 2>&1 &
  local pid=$!
  while kill -0 "${pid}" 2>/dev/null; do
    sleep 1
    printf "."
  done
  local end_time=$(date +%s.%N)
  local elapsed=$(awk "BEGIN {printf \"%.3f\", ${end_time} - ${start_time}}")
  wait "${pid}"
  local exit_code=$?
  if [ ${exit_code} -eq 0 ]; then
    echo " done (${elapsed}s)"
  fi
  return ${exit_code}
}

# Extract and display relevant error information from build log
show_error_log() {
  local log_file="$1"
  local error_block
  error_block=$(awk '
    /^FAILED:/ { printing=1; next }
    /^\[[0-9]+\/[0-9]+\]/ { printing=0 }
    /^ninja: build stopped/ { printing=0 }
    printing && /\.(exe|clang|gcc) / { next }
    printing { print }
  ' "${log_file}" 2>/dev/null | head -n 50)

  if [ -n "${error_block}" ]; then
    echo "${error_block}"
  else
    tail -n 30 "${log_file}"
  fi
  echo ""
  echo "(Full log: ${log_file})"
}

# Extract and display relevant test failure information from JUnit XML
show_test_log() {
  local log_file="$1"
  local xml_file="$(dirname "${log_file}")/testlog.xml"

  if [ -f "${xml_file}" ]; then
    awk '
      /<testsuite/,/>/ {
        if (/tests="/) { match($0, /tests="([0-9]+)"/, t); tests = t[1] }
        if (/failures="/) { match($0, /failures="([0-9]+)"/, f); failures = f[1] }
      }
      /<testcase / {
        match($0, /name="([^"]*)"/, arr)
        testname = arr[1]
        has_failure = 0
        system_out = ""
      }
      /<failure/ { has_failure = 1 }
      /<system-out>/ {
        in_system_out = 1
        gsub(/.*<system-out>/, "")
      }
      in_system_out {
        if (/<\/system-out>/) {
          gsub(/<\/system-out>.*/, "")
          system_out = system_out $0
          in_system_out = 0
        } else {
          system_out = system_out $0 "\n"
        }
      }
      /<\/testcase>/ {
        if (has_failure) {
          print "=== " testname " ==="
          print system_out
        }
      }
      END {
        if (failures > 0) {
          printf "%d/%d tests failed\n", failures, tests
        }
      }
    ' "${xml_file}"
    echo ""
    echo "(Full log: ${log_file})"
  else
    tail -n 30 "${log_file}"
    echo ""
    echo "(Full log: ${log_file})"
  fi
}

if [ "${INSTALL_TOOLS}" -eq 1 ]; then
  mkdir -p build/tools
  . setup-llvm-mingw.sh --dir "${PWD}/build/tools"
fi

ARCHDIR="${ARCHDIR:-$(uname -m)}"
destdir="${PWD}/build/${CMAKE_PRESET}/${ARCHDIR}"
build_log="${destdir}/build.log"
test_log="${destdir}/test.log"
install_log="${destdir}/install.log"
CMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX:-${destdir}/local}"
CMAKE_C_COMPILER="${CMAKE_C_COMPILER:-clang}"
CMAKE_TOOLCHAIN_FILE="${CMAKE_TOOLCHAIN_FILE:-}"

if [ "${REBUILD}" -eq 1 ] || [ ! -e "${destdir}/CMakeCache.txt" ]; then
  rm -rf "${destdir}"
  cmake -S . -B "${destdir}" --preset "${CMAKE_PRESET}" \
    -DFORMAT_SOURCES="${FORMAT_SOURCES}" \
    -DCMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}" \
    -DCMAKE_C_COMPILER="${CMAKE_C_COMPILER}" \
    -DCMAKE_TOOLCHAIN_FILE="${CMAKE_TOOLCHAIN_FILE}" > /dev/null 2>&1
fi

if ! run_with_progress "Building" "${build_log}" cmake --build "${destdir}"; then
  echo ""
  echo "Build failed:"
  show_error_log "${build_log}"
  exit 1
fi

if [ "${SKIP_TESTS}" -eq 0 ]; then
  if ! run_with_progress "Testing" "${test_log}" ctest --test-dir "${destdir}" --output-on-failure --output-junit testlog.xml; then
    echo ""
    echo "Tests failed:"
    show_test_log "${test_log}"
    exit 1
  fi
fi

if ! run_with_progress "Installing" "${install_log}" cmake --install "${destdir}"; then
  echo ""
  echo "Install failed:"
  show_error_log "${install_log}"
  exit 1
fi

echo "Build successful."
cd "${CUR_DIR}"
