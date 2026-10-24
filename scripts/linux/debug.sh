#!/bin/bash
# Launches CuteGit under gdb with automatic backtrace capture on crash.
# Usage: ./debug.sh [--debug|--release] [--offscreen] (single command, no gdb typing needed)
# Logs: tmp/debug.log (full session), tmp/debug-bt.log (backtrace only).
set -euo pipefail

SCRIPT_FOLDER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SCRIPT_FOLDER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT_FOLDER="$(cd "${SCRIPT_FOLDER}/../.." && pwd)"

EXIT_SUCCESS=0
EXIT_FAILURE=1

BUILD_TYPE="Release"
OFFSCREEN=0

usage() {
  cat <<'USAGE'
Usage: ./debug.sh [--debug|--release] [-d|-r] [-c Debug|Release] [--offscreen]
  --debug,   -d  Debug binary CuteGitApp-dbg
  --release, -r  Release binary CuteGitApp (default, matches the running build)
  --offscreen     QT_QPA_PLATFORM=offscreen (headless check, no window)
  -c              Build type (Debug or Release)
  -h, --help      Show this help
Logs:
  tmp/debug.log     full gdb session (copy-paste on crash)
  tmp/debug-bt.log  backtrace only
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
    --offscreen)
      OFFSCREEN=1
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

if ! command -v gdb >/dev/null 2>&1; then
  echo "[debug] gdb not found. Install gdb to capture backtraces." >&2
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
  echo "[debug] Executable not found: ${EXECUTABLE_PATH}" >&2
  echo "[debug] Run ./build.sh first (do not rebuild a crashing version before diagnosis)." >&2
  exit "${EXIT_FAILURE}"
fi

if pgrep -f "[C]uteGitApp(-dbg)?" >/dev/null 2>&1; then
  echo "[debug] A CuteGit instance is already running (single-instance shared memory)." >&2
  echo "[debug] Quit it first, then rerun $0, so the gdb instance can start." >&2
  exit "${EXIT_FAILURE}"
fi

LOG_FOLDER="${PROJECT_ROOT_FOLDER}/tmp"
mkdir -p "${LOG_FOLDER}"
SESSION_LOG="${LOG_FOLDER}/debug.log"
BT_LOG="${LOG_FOLDER}/debug-bt.log"
: > "${SESSION_LOG}"
: > "${BT_LOG}"

echo "[debug] Binary: ${EXECUTABLE_PATH}"
file "${EXECUTABLE_PATH}" | sed 's/^/[debug] /' || true
if command -v git >/dev/null 2>&1 && git -C "${PROJECT_ROOT_FOLDER}" rev-parse --short HEAD >/dev/null 2>&1; then
  echo "[debug] git HEAD: $(git -C "${PROJECT_ROOT_FOLDER}" rev-parse --short HEAD) $(git -C "${PROJECT_ROOT_FOLDER}" log -1 --format=%ad --date=short)"
fi
echo "[debug] Session log: ${SESSION_LOG}"
echo "[debug] Backtrace log: ${BT_LOG}"
echo "[debug] Reproduce the crash in the CuteGit window. Backtrace is captured automatically."
echo "[debug] If the app hangs instead of crashing, run ./scripts/linux/hang.sh from another terminal (SIGABRT dumps stacks; SIGKILL leaves no trace)."

ulimit -c unlimited || true

export LD_LIBRARY_PATH="${PROJECT_ROOT_FOLDER}/qt-plus/bin"
export QT_MESSAGE_PATTERN="%{time process} %{type} %{category} %{function}:%{line} - %{message}"
if [[ "${OFFSCREEN}" -eq 1 ]]; then
  export QT_QPA_PLATFORM=offscreen
  echo "[debug] Offscreen mode enabled."
fi

set +e
gdb \
  -ex "set pagination off" \
  -ex "set logging file ${BT_LOG}" \
  -ex "set logging enabled on" \
  -ex "handle SIGSEGV stop print nopass" \
  -ex "handle SIGABRT stop print nopass" \
  -ex run \
  -ex "thread apply all bt full" \
  -ex "info sharedlibrary" \
  -ex "info registers" \
  -ex "set logging enabled off" \
  -ex "quit" \
  --args "${EXECUTABLE_PATH}" 2>&1 | tee "${SESSION_LOG}"
GDB_CODE="${PIPESTATUS[0]}"
set -e

echo "[debug] Done (gdb exit ${GDB_CODE})."
echo "[debug] Send ${SESSION_LOG} and ${BT_LOG} for diagnosis."
exit "${GDB_CODE}"
