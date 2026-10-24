#!/bin/bash
set -euo pipefail

SCRIPT_FOLDER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT_FOLDER="$(cd "${SCRIPT_FOLDER}/../.." && pwd)"

EXIT_SUCCESS=0
EXIT_FAILURE=1
VERBOSE_ENABLED=1
VERBOSE_DISABLED=0

BUILD_TYPE="Release"
VERBOSE_BUILD="${VERBOSE_DISABLED}"

usage() {
  cat <<'USAGE'
Usage: ./build.sh [--debug|--release] [-d|-r] [--verbose|-v] [-c Debug|Release] [-h|--help]
  --debug,   -d  Build type Debug
  --release, -r  Build type Release (default)
  --verbose, -v  Verbose build output
  -c             Build type (Debug or Release)
USAGE
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --debug|-d)
      BUILD_TYPE="Debug"
      shift
      ;;
    --release|-r)
      BUILD_TYPE="Release"
      shift
      ;;
    --verbose|-v)
      VERBOSE_BUILD="${VERBOSE_ENABLED}"
      shift
      ;;
    -c)
      if [[ -z "${2:-}" ]]; then
        echo "Option -c requires an argument." >&2
        usage >&2
        exit "${EXIT_FAILURE}"
      fi
      BUILD_TYPE="$2"
      shift 2
      ;;
    -h|--help)
      usage
      exit "${EXIT_SUCCESS}"
      ;;
    *)
      echo "Unknown option: ${1}" >&2
      usage >&2
      exit "${EXIT_FAILURE}"
      ;;
  esac
done

if [[ "${BUILD_TYPE}" != "Debug" && "${BUILD_TYPE}" != "Release" ]]; then
  echo "Invalid build type: ${BUILD_TYPE} (use Debug or Release)" >&2
  exit "${EXIT_FAILURE}"
fi

BUILD_DIR="${PROJECT_ROOT_FOLDER}/build-${BUILD_TYPE,,}"

if [ ! -f "${PROJECT_ROOT_FOLDER}/qt-plus/source/cpp/CXMLNode.cpp" ] \
  || [ ! -f "${PROJECT_ROOT_FOLDER}/qt-plus/source/cpp/CFastList.h" ]; then
  echo "[build] Submodules missing, run ./scripts/linux/deps.sh first." >&2
  exit "${EXIT_FAILURE}"
fi

# Find CMake and Ninja
if ! command -v cmake >/dev/null 2>&1; then
  echo "[build] cmake not found, run ./scripts/linux/deps.sh first." >&2
  exit "${EXIT_FAILURE}"
fi

if ! command -v ninja >/dev/null 2>&1; then
  echo "[build] ninja not found, run ./scripts/linux/deps.sh first." >&2
  exit "${EXIT_FAILURE}"
fi

echo "Using cmake: $(command -v cmake)"

# --- BEGIN cutegit-only extras (BuildInfo stamp, layouts.json) ---
# The qt-plus sources this project needs (CXMLNode.cpp, CFastList.h) are
# compiled directly into the binary, so no qt-plus sub-build is required.

# Build stamp: regenerated on every build so a running binary can always be identified
# Written to the build folder (never to the source tree), consumed via the binary dir include path
# main.cpp is touched so the new stamp is always compiled in
BUILD_STAMP="$(date -u +%Y-%m-%dT%H:%M:%SZ)"
mkdir -p "${BUILD_DIR}" "${BUILD_DIR}/CuteGit"
printf '#pragma once\n#define BUILD_TIMESTAMP "%s"\n' "${BUILD_STAMP}" > "${BUILD_DIR}/CuteGit/BuildInfo.h"
touch "${PROJECT_ROOT_FOLDER}/CuteGit/sources/main.cpp"
echo "Build stamp: ${BUILD_STAMP}"
# --- END cutegit-only extras ---

echo "Running cmake with build type ${BUILD_TYPE}..."
cmake -S "${PROJECT_ROOT_FOLDER}" -B "${BUILD_DIR}" -G Ninja "-DCMAKE_BUILD_TYPE=${BUILD_TYPE}"

if [ "${VERBOSE_BUILD}" -eq "${VERBOSE_ENABLED}" ]; then
  cmake --build "${BUILD_DIR}" -v
else
  cmake --build "${BUILD_DIR}"
fi

# --- BEGIN cutegit-only extras (layouts.json copy) ---
# Descriptive UI layouts live next to the binary
if [[ -f "${PROJECT_ROOT_FOLDER}/CuteGit/deploy/layouts.json" ]]; then
  cp "${PROJECT_ROOT_FOLDER}/CuteGit/deploy/layouts.json" "${BUILD_DIR}/CuteGit/bin/layouts.json"
  echo "Copied layouts.json next to the binary."
fi
# --- END cutegit-only extras ---

printf '\nBuilt target in %s\n' "${BUILD_DIR}"
