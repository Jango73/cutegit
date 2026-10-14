#!/bin/bash
# Builds and runs the automated test sections.
# Usage: ./tests.sh [--debug|--release] [-d|-r] [section ...]
# Sections run independently; default runs all of them (currently: diff).
set -euo pipefail

SCRIPT_FOLDER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_FOLDER="$(cd "${SCRIPT_FOLDER}/../.." && pwd)"

EXIT_SUCCESS=0
EXIT_FAILURE=1

BUILD_TYPE="Debug"
SECTIONS=""

usage() {
  cat <<'USAGE'
Usage: ./tests.sh [--debug|--release] [-d|-r] [section ...]
  --debug,   -d  Build type Debug (default)
  --release, -r  Build type Release
  section         Test section to run (default: all, currently: diff repository startup)
  -h, --help      Show this help
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
    -h|--help)
      usage
      exit "${EXIT_SUCCESS}"
      ;;
    -*)
      echo "Unknown option: $1" >&2
      usage >&2
      exit "${EXIT_FAILURE}"
      ;;
    *)
      SECTIONS="${SECTIONS} $1"
      shift
      ;;
  esac
done

if [[ -z "${SECTIONS}" ]]; then
  SECTIONS="diff startup repository"
fi

BUILD_TYPE_LOWER="$(echo "${BUILD_TYPE}" | tr '[:upper:]' '[:lower:]')"

FAILED=0

build_tests() {
  local tests_folder="${ROOT_FOLDER}/tests"
  local build_folder="${tests_folder}/build-${BUILD_TYPE_LOWER}"

  echo "[tests] Creating diff fixture repository..."
  "${tests_folder}/create-diff-fixture.sh" > /dev/null

  echo "[tests] Building test sections (${BUILD_TYPE_LOWER})..."
  mkdir -p "${build_folder}"
  ( cd "${build_folder}" && qmake6 "${tests_folder}/diff.pro" "CONFIG+=${BUILD_TYPE_LOWER}" > /dev/null && make -j "$(nproc)" )

  # Layouts next to the test binaries, like build.sh does for the application
  if [[ -f "${ROOT_FOLDER}/CuteGit/deploy/layouts.json" ]]; then
    cp "${ROOT_FOLDER}/CuteGit/deploy/layouts.json" "${tests_folder}/bin/layouts.json"
  fi
}

run_diff_section() {
  local tests_folder="${ROOT_FOLDER}/tests"
  local bin_folder="${tests_folder}/bin"
  local fixture_folder="${tests_folder}/fixtures/diff/repo"

  echo "[tests] Running tst_patchbuilder..."
  DIFF_FIXTURE_REPO="${fixture_folder}" QT_QPA_PLATFORM=offscreen \
    "${bin_folder}/tst_patchbuilder" || FAILED=1

  echo "[tests] Running tst_DiffView.qml..."
  ( cd "${tests_folder}" && QT_QPA_PLATFORM=offscreen \
    "${bin_folder}/diff-qmlsetup" -input qml/tst_DiffView.qml ) || FAILED=1
}

run_repository_section() {
  local tests_folder="${ROOT_FOLDER}/tests"
  local bin_folder="${tests_folder}/bin"
  local fixture_folder="${tests_folder}/fixtures/diff/repo"

  echo "[tests] Running tst_repository..."
  DIFF_FIXTURE_REPO="${fixture_folder}" QT_QPA_PLATFORM=offscreen \
    "${bin_folder}/tst_repository" || FAILED=1
}

run_startup_section() {
  local tests_folder="${ROOT_FOLDER}/tests"
  local bin_folder="${tests_folder}/bin"
  local fixture_folder="${tests_folder}/fixtures/diff/repo"

  echo "[tests] Running tst_startup..."
  DIFF_FIXTURE_REPO="${fixture_folder}" QT_QPA_PLATFORM=offscreen \
    "${bin_folder}/tst_startup" || FAILED=1
}

for section in ${SECTIONS}; do
  case "${section}" in
    diff|repository|startup)
      ;;
    *)
      echo "[tests] Unknown section: ${section}" >&2
      exit "${EXIT_FAILURE}"
      ;;
  esac
done

build_tests

for section in ${SECTIONS}; do
  case "${section}" in
    diff)
      run_diff_section
      ;;
    repository)
      run_repository_section
      ;;
    startup)
      run_startup_section
      ;;
  esac
done

if [[ "${FAILED}" -ne 0 ]]; then
  echo "[tests] FAILURES detected." >&2
  exit "${EXIT_FAILURE}"
fi

echo "[tests] All sections passed."
exit "${EXIT_SUCCESS}"
