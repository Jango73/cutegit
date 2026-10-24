#!/bin/bash
# Launches the CuteGit binary N times, 5 seconds per run, offscreen.
# Stops at the first crash (exit code other than 124 = timeout kill).
# Usage: ./crashloop.sh [N] [--debug|--release]
set -uo pipefail

SCRIPT_FOLDER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SCRIPT_FOLDER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT_FOLDER="$(cd "${SCRIPT_FOLDER}/../.." && pwd)"

RUN_COUNT=100
BUILD_TYPE="Release"

for arg in "$@"; do
  case "${arg}" in
    --debug) BUILD_TYPE="Debug" ;;
    --release) BUILD_TYPE="Release" ;;
    *)
      if [[ "${arg}" =~ ^[0-9]+$ ]]; then
        RUN_COUNT="${arg}"
      else
        echo "Unknown argument: ${arg}" >&2
        echo "Usage: $0 [N] [--debug|--release]" >&2
        exit 1
      fi
      ;;
  esac
done

BUILD_FOLDER="${PROJECT_ROOT_FOLDER}/build-${BUILD_TYPE,,}"

if [[ "${BUILD_TYPE}" == "Debug" ]]; then
  BINARY="${BUILD_FOLDER}/CuteGit/bin/CuteGitApp-dbg"
else
  BINARY="${BUILD_FOLDER}/CuteGit/bin/CuteGitApp"
fi

if [ ! -f "${BINARY}" ]; then
  echo "[crashloop] Binary not found: ${BINARY}" >&2
  exit 1
fi

LOG_FOLDER="/tmp/opencode"
mkdir -p "${LOG_FOLDER}"

echo "[crashloop] ${RUN_COUNT} runs x 5s, binary ${BINARY}"

for i in $(seq 1 "${RUN_COUNT}"); do
  QT_QPA_PLATFORM=offscreen \
    LD_LIBRARY_PATH="${PROJECT_ROOT_FOLDER}/qt-plus/bin" \
    timeout 5s "${BINARY}" > "${LOG_FOLDER}/crashloop-${i}.log" 2>&1
  code="$?"
  echo "run ${i} exit:${code}"

  if [ "${code}" -ne 124 ]; then
    echo "[crashloop] CRASH at run ${i} (exit ${code}). Log:"
    cat "${LOG_FOLDER}/crashloop-${i}.log"
    exit "${code}"
  fi
done

echo "[crashloop] No crash in ${RUN_COUNT} runs."
