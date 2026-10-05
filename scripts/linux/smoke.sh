#!/bin/bash
# Headless smoke test for the CuteGit GUI application.
# Starts the binary offscreen, waits a few seconds, then kills it.
# Success = the application stays alive (exit code 124 from timeout).
# Any other exit code (crash, missing library, QML error at startup) = failure.
set -euo pipefail

ROOT_FOLDER="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

EXIT_SUCCESS=0
EXIT_FAILURE=1

BUILD_TYPE="Debug"
WAIT_SECONDS=8

usage() {
  cat <<'USAGE'
Usage: ./smoke.sh [--debug|--release] [-d|-r] [-c Debug|Release] [--wait SECONDS]
  --debug,   -d  Smoke test the Debug binary (default)
  --release, -r  Smoke test the Release binary
  -c             Build type (Debug or Release)
  --wait         Seconds to keep the application alive (default: 8, max: 15)
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
    --wait)
      if [[ -z "${2:-}" ]]; then
        echo "Option --wait requires an argument." >&2
        usage >&2
        exit "${EXIT_FAILURE}"
      fi
      WAIT_SECONDS="$2"
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

if [[ "${WAIT_SECONDS}" -gt 15 ]]; then
  echo "Wait time capped at 15 seconds." >&2
  WAIT_SECONDS=15
fi

BUILD_FOLDER="${ROOT_FOLDER}/build-${BUILD_TYPE,,}"

if [[ "${BUILD_TYPE}" == "Debug" ]]; then
  BINARY="${BUILD_FOLDER}/CuteGit/bin/CuteGitApp-dbg"
else
  BINARY="${BUILD_FOLDER}/CuteGit/bin/CuteGitApp"
fi

if [ ! -f "${BINARY}" ]; then
  echo "[smoke] Binary not found at ${BINARY}, run ./build.sh first." >&2
  exit "${EXIT_FAILURE}"
fi

echo "[smoke] Starting ${BINARY} offscreen for ${WAIT_SECONDS}s..."
if QT_QPA_PLATFORM=offscreen \
  LD_LIBRARY_PATH="${ROOT_FOLDER}/qt-plus/bin" \
  timeout "${WAIT_SECONDS}s" "${BINARY}" > /tmp/cutegit-smoke.log 2>&1; then
  echo "[smoke] UNEXPECTED: application exited on its own (code 0)." >&2
  exit "${EXIT_FAILURE}"
else
  code="$?"
  if [ "${code}" -eq 124 ]; then
    echo "[smoke] OK: application stayed alive for ${WAIT_SECONDS}s."
    exit "${EXIT_SUCCESS}"
  fi
  echo "[smoke] FAILURE: application exited with code ${code}. Log:" >&2
  tail -20 /tmp/cutegit-smoke.log >&2
  exit "${EXIT_FAILURE}"
fi
