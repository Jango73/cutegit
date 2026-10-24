#!/bin/bash
# Local mirror of the GitHub CI build: builds the project in release mode
# and runs the unit tests. Run this before pushing, it catches failures
# here instead of burning CI runs.
set -euo pipefail

SCRIPT_FOLDER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT_FOLDER="$(cd "${SCRIPT_FOLDER}/../.." && pwd)"

EXIT_SUCCESS=0
EXIT_FAILURE=1

usage() {
  cat <<'USAGE'
Usage: ./ci-local.sh [--debug|--release] [-d|-r] [-c Debug|Release] [-h|--help]
  --debug,   -d  Build type Debug
  --release, -r  Build type Release (default, same as CI)
  -c             Build type (Debug or Release)
USAGE
}

BUILD_TYPE="Release"

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

echo "[ci-local] Building (${BUILD_TYPE})..."
"${SCRIPT_FOLDER}/build.sh" -c "${BUILD_TYPE}"

BUILD_TYPE_LOWER="$(echo "${BUILD_TYPE}" | tr '[:upper:]' '[:lower:]')"
TESTS_FOLDER="${PROJECT_ROOT_FOLDER}/tests"
TESTS_BUILD_FOLDER="${TESTS_FOLDER}/build-${BUILD_TYPE_LOWER}"
TESTS_BIN_FOLDER="${TESTS_FOLDER}/bin"
FIXTURE_REPO="${TESTS_FOLDER}/fixtures/diff/repo"
LOG_FILE="${PROJECT_ROOT_FOLDER}/tmp/ci-local.log"

mkdir -p "$(dirname "${LOG_FILE}")"

echo "[ci-local] Creating diff fixture repository..."
"${TESTS_FOLDER}/create-diff-fixture.sh" > /dev/null

echo "[ci-local] Building test sections (${BUILD_TYPE_LOWER})..."
mkdir -p "${TESTS_BUILD_FOLDER}"
cmake -S "${TESTS_FOLDER}" -B "${TESTS_BUILD_FOLDER}" -G Ninja "-DCMAKE_BUILD_TYPE=${BUILD_TYPE}" > /dev/null
cmake --build "${TESTS_BUILD_FOLDER}" -j "$(nproc)"

if [[ -f "${PROJECT_ROOT_FOLDER}/CuteGit/deploy/layouts.json" ]]; then
  cp "${PROJECT_ROOT_FOLDER}/CuteGit/deploy/layouts.json" "${TESTS_BIN_FOLDER}/layouts.json"
fi

echo "[ci-local] Running tests (logging to ${LOG_FILE})..."
{
  echo "[ci-local] Running tst_patchbuilder..."
  DIFF_FIXTURE_REPO="${FIXTURE_REPO}" QT_QPA_PLATFORM=offscreen \
    "${TESTS_BIN_FOLDER}/tst_patchbuilder"
  echo "[ci-local] Running tst_DiffView.qml..."
  ( cd "${TESTS_FOLDER}" && QT_QPA_PLATFORM=offscreen \
    "${TESTS_BIN_FOLDER}/diff-qmlsetup" -input qml/tst_DiffView.qml )
  echo "[ci-local] Running tst_startup..."
  DIFF_FIXTURE_REPO="${FIXTURE_REPO}" QT_QPA_PLATFORM=offscreen \
    "${TESTS_BIN_FOLDER}/tst_startup"
  echo "[ci-local] Running tst_repository..."
  DIFF_FIXTURE_REPO="${FIXTURE_REPO}" QT_QPA_PLATFORM=offscreen \
    "${TESTS_BIN_FOLDER}/tst_repository"
} 2>&1 | tee "${LOG_FILE}"

echo "[ci-local] Local CI check passed."
