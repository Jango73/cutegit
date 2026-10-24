#!/bin/bash
# Traces every child process spawned by CuteGit (fork/clone/execve with timestamps).
# Usage: ./strace.sh [--debug|--release] [-d|-r] [-c Debug|Release]
# Log: tmp/strace.log (one line per spawn, with executable and arguments,
# plus working-folder changes so each git maps to its repository).
# Reproduce the hang, then quit or kill the application, and share the log.
set -euo pipefail

SCRIPT_FOLDER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SCRIPT_FOLDER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT_FOLDER="$(cd "${SCRIPT_FOLDER}/../.." && pwd)"

EXIT_SUCCESS=0
EXIT_FAILURE=1

BUILD_TYPE="Release"

usage() {
  cat <<'USAGE'
Usage: ./strace.sh [--debug|--release] [-d|-r] [-c Debug|Release]
  --debug,   -d  Trace the Debug binary CuteGitApp-dbg
  --release, -r  Trace the Release binary CuteGitApp (default)
  -c              Build type (Debug or Release)
  -h, --help      Show this help
Log:
  tmp/strace.log  fork/clone/execve trace with timestamps
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
      echo "Unknown option: $1" >&2
      usage >&2
      exit "${EXIT_FAILURE}"
      ;;
  esac
done

if [[ "${BUILD_TYPE}" != "Debug" && "${BUILD_TYPE}" != "Release" ]]; then
  echo "Invalid build type: ${BUILD_TYPE} (use Debug or Release)" >&2
  exit "${EXIT_FAILURE}"
fi

if ! command -v strace >/dev/null 2>&1; then
  echo "[strace] strace not found. Install strace to trace process spawns." >&2
  exit "${EXIT_FAILURE}"
fi

BUILD_FOLDER="${PROJECT_ROOT_FOLDER}/build-${BUILD_TYPE,,}"

if [[ "${BUILD_TYPE}" == "Debug" ]]; then
  EXECUTABLE_NAME="CuteGitApp-dbg"
else
  EXECUTABLE_NAME="CuteGitApp"
fi

EXECUTABLE_PATH="${BUILD_FOLDER}/CuteGit/bin/${EXECUTABLE_NAME}"

if [[ ! -f "${EXECUTABLE_PATH}" ]]; then
  echo "[strace] Executable not found: ${EXECUTABLE_PATH}" >&2
  echo "[strace] Run ./build.sh first." >&2
  exit "${EXIT_FAILURE}"
fi

if pgrep -f "[C]uteGitApp(-dbg)?" >/dev/null 2>&1; then
  echo "[strace] A CuteGit instance is already running (single-instance shared memory)." >&2
  echo "[strace] Quit it first, then rerun $0." >&2
  exit "${EXIT_FAILURE}"
fi

LOG_FOLDER="${PROJECT_ROOT_FOLDER}/tmp"
mkdir -p "${LOG_FOLDER}"
TRACE_LOG="${LOG_FOLDER}/strace.log"
: > "${TRACE_LOG}"

echo "[strace] Binary: ${EXECUTABLE_PATH}"
echo "[strace] Trace log: ${TRACE_LOG}"
echo "[strace] Reproduce the hang, then quit or kill the application."

export LD_LIBRARY_PATH="${PROJECT_ROOT_FOLDER}/qt-plus/bin"
exec strace -f -tt -T -e trace=process,chdir -o "${TRACE_LOG}" "${EXECUTABLE_PATH}"
