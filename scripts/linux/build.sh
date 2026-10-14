#!/bin/bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

EXIT_SUCCESS=0
EXIT_FAILURE=1
VERBOSE_ENABLED=1
VERBOSE_DISABLED=0

BUILD_TYPE="Release"
VERBOSE_BUILD="${VERBOSE_DISABLED}"

usage() {
  cat <<'USAGE'
Usage: ./build.sh [--debug|--release] [-d|-r] [--verbose|-v] [-c Debug|Release]
  --debug,   -d  Build type Debug (default)
  --release, -r  Build type Release
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

# Find qmake executable (prefer Qt6)
QMAKE=""
for candidate in qmake6 qmake-qt6 qmake; do
  if command -v "${candidate}" >/dev/null 2>&1; then
    QMAKE="${candidate}"
    break
  fi
done

if [[ -z "${QMAKE}" ]]; then
  echo "Error: qmake not found. Install Qt development tools." >&2
  exit 1
fi

echo "Using qmake: ${QMAKE}"

BUILD_DIR="${ROOT_DIR}/build-${BUILD_TYPE,,}"

# Build qt-plus if it exists
QT_PLUS_DIR="${ROOT_DIR}/qt-plus"
if [[ -d "${QT_PLUS_DIR}" && -f "${QT_PLUS_DIR}/build.sh" ]]; then
  echo "Building qt-plus library..."
  if [ "${VERBOSE_BUILD}" -eq "${VERBOSE_ENABLED}" ]; then
    "${QT_PLUS_DIR}/build.sh" -c "${BUILD_TYPE}" -v
  else
    "${QT_PLUS_DIR}/build.sh" -c "${BUILD_TYPE}"
  fi
  echo "qt-plus built successfully."
else
  echo "Warning: qt-plus not found or missing build.sh" >&2
fi

# Build CuteGit
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Build stamp: regenerated on every build so a running binary can always be identified
# Written to the build folder (never to the source tree), consumed via INCLUDEPATH += $$OUT_PWD
# main.cpp is touched so the new stamp is always compiled in
BUILD_STAMP="$(date -u +%Y-%m-%dT%H:%M:%SZ)"
mkdir -p "${BUILD_DIR}/CuteGit"
printf '#pragma once\n#define BUILD_TIMESTAMP "%s"\n' "${BUILD_STAMP}" > "${BUILD_DIR}/CuteGit/BuildInfo.h"
touch "${ROOT_DIR}/CuteGit/sources/main.cpp"
echo "Build stamp: ${BUILD_STAMP}"

if [[ "${BUILD_TYPE}" == "Debug" ]]; then
  CONFIG_ARG="CONFIG+=debug"
else
  CONFIG_ARG="CONFIG+=release"
fi

echo "Running qmake with ${CONFIG_ARG}..."
"${QMAKE}" "${ROOT_DIR}/CuteGit.pro" "${CONFIG_ARG}"

if [ "${VERBOSE_BUILD}" -eq "${VERBOSE_ENABLED}" ]; then
  make -j"$(nproc)" VERBOSE=1
else
  make -j"$(nproc)"
fi

# Descriptive UI layouts live next to the binary
if [ -f "${ROOT_DIR}/CuteGit/deploy/layouts.json" ]; then
  cp "${ROOT_DIR}/CuteGit/deploy/layouts.json" "${BUILD_DIR}/CuteGit/bin/layouts.json"
  echo "Copied layouts.json next to the binary."
fi

printf '\nBuilt target in %s\n' "${BUILD_DIR}"
