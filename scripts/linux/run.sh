#!/bin/bash
set -euo pipefail

SCRIPT_FOLDER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_FOLDER="$(cd "${SCRIPT_FOLDER}/../.." && pwd)"

EXIT_SUCCESS=0
EXIT_FAILURE=1

BUILD_TYPE="Release"

usage() {
  cat <<'USAGE'
Usage: ./run.sh [--debug|--release] [-d|-r] [-c Debug|Release]
  --debug,   -d  Build type Debug (default)
  --release, -r  Build type Release
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

BUILD_FOLDER="${ROOT_FOLDER}/build-${BUILD_TYPE,,}"
LOG_FILE="${ROOT_FOLDER}/tmp/run.log"

if [ ! -d "${BUILD_FOLDER}" ]; then
  echo "[run] ${BUILD_FOLDER} not found, run ./build.sh first." >&2
  exit "${EXIT_FAILURE}"
fi

if [[ "${BUILD_TYPE}" == "Debug" ]]; then
  EXECUTABLE_NAME="CuteGitApp-dbg"
else
  EXECUTABLE_NAME="CuteGitApp"
fi

EXECUTABLE_PATH="${BUILD_FOLDER}/CuteGit/bin/${EXECUTABLE_NAME}"

if [ ! -f "${EXECUTABLE_PATH}" ]; then
  echo "[run] Executable not found: ${EXECUTABLE_PATH}" >&2
  exit 1
fi

mkdir -p "$(dirname "${LOG_FILE}")"
echo "[run] Logging to ${LOG_FILE}"
LD_LIBRARY_PATH="${ROOT_FOLDER}/qt-plus/bin" \
  exec "${EXECUTABLE_PATH}" 2>&1 | tee "${LOG_FILE}"
