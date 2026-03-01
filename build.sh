#!/bin/bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

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

if [[ "${BUILD_TYPE}" == "Debug" ]]; then
  CONFIG_ARG="CONFIG+=debug"
else
  CONFIG_ARG="CONFIG+=release"
fi

echo "Running qmake with ${CONFIG_ARG}..."
qmake "${ROOT_DIR}/CuteGit.pro" "${CONFIG_ARG}"

if [ "${VERBOSE_BUILD}" -eq "${VERBOSE_ENABLED}" ]; then
  make VERBOSE=1
else
  make
fi

printf '\nBuilt target in %s\n' "${BUILD_DIR}"
