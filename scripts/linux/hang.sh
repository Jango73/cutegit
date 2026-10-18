#!/bin/bash
# Sends SIGABRT to a hung CuteGit instance running under scripts/linux/debug.sh
# so gdb dumps "thread apply all bt full" into tmp/debug-bt.log.
# Usage: ./hang.sh
# A SIGKILL (forcer l'arret) leaves no stack trace; SIGABRT does.
set -euo pipefail

SCRIPT_FOLDER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_FOLDER="$(cd "${SCRIPT_FOLDER}/../.." && pwd)"

EXIT_SUCCESS=0
EXIT_FAILURE=1

FOUND=0
for PROCESS_NAME in CuteGitApp CuteGitApp-dbg; do
  while read -r PID; do
    [[ -z "${PID}" ]] && continue
    FOUND=1
    PARENT_PID="$(ps -o ppid= -p "${PID}" 2>/dev/null | tr -d ' ' || true)"
    PARENT_NAME="$(ps -o comm= -p "${PARENT_PID}" 2>/dev/null | tr -d ' ' || true)"
    if [[ "${PARENT_NAME}" == "gdb" ]]; then
      echo "[hang] Sending SIGABRT to ${PROCESS_NAME} (pid ${PID}, parent is gdb)."
      kill -ABRT "${PID}"
      echo "[hang] Done. Wait for the gdb session to dump stacks, then send tmp/debug.log and tmp/debug-bt.log for diagnosis."
    else
      echo "[hang] pid ${PID} (${PROCESS_NAME}) is not running under gdb (parent: ${PARENT_NAME:-unknown})." >&2
      echo "[hang] SIGABRT would kill it without a stack trace. Rerun it via ./scripts/linux/debug.sh first, then rerun $0." >&2
      exit "${EXIT_FAILURE}"
    fi
  done < <(pgrep -x "${PROCESS_NAME}" || true)
done

if [[ "${FOUND}" -eq 0 ]]; then
  echo "[hang] No CuteGitApp process found." >&2
  exit "${EXIT_FAILURE}"
fi

exit "${EXIT_SUCCESS}"
