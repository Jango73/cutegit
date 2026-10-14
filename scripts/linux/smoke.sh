#!/bin/bash
# Smoke test for the CuteGit GUI application.
# Builds the application, then runs the automated test sections.
# Success = build passes and all test sections pass.
set -euo pipefail

ROOT_FOLDER="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SCRIPT_FOLDER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

EXIT_SUCCESS=0
EXIT_FAILURE=1

BUILD_TYPE="Debug"

usage() {
  cat <<'USAGE'
Usage: ./smoke.sh [--debug|--release] [-d|-r] [-c Debug|Release]
  --debug,   -d  Build and test the Debug binaries (default)
  --release, -r  Build and test the Release binaries
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

if [[ "${BUILD_TYPE}" == "Debug" ]]; then
  TYPE_FLAG="--debug"
else
  TYPE_FLAG="--release"
fi

echo "[smoke] Building the application (${BUILD_TYPE})..."
"${SCRIPT_FOLDER}/build.sh" "${TYPE_FLAG}"

echo "[smoke] Running the automated tests (${BUILD_TYPE})..."
"${SCRIPT_FOLDER}/tests.sh" "${TYPE_FLAG}"

echo "[smoke] OK: build passed and all test sections passed."
