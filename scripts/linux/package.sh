#!/bin/bash
# Builds Release packages from an isolated build folder, then copies the
# versioned artifacts into dist/.
# Produces a .deb and a .tar.gz (CPack DEB;TGZ generators).
set -euo pipefail

SCRIPT_FOLDER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT_FOLDER="$(cd "${SCRIPT_FOLDER}/../.." && pwd)"
BUILD_TYPE="Release"
BUILD_FOLDER="${PROJECT_ROOT_FOLDER}/build-package-${BUILD_TYPE,,}"
OUTPUT_FOLDER="${PROJECT_ROOT_FOLDER}/dist"
VERSION="$(tr -d '[:space:]' < "${PROJECT_ROOT_FOLDER}/VERSION")"

EXIT_SUCCESS=0
EXIT_FAILURE=1

usage() {
  cat <<'USAGE'
Usage: ./package.sh [-h|--help]
  Build Release packages (.deb + .tar.gz) into dist/.
USAGE
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
  usage
  exit "${EXIT_SUCCESS}"
fi

if [[ $# -ne 0 ]]; then
  usage >&2
  exit "${EXIT_FAILURE}"
fi

if ! command -v cmake >/dev/null 2>&1; then
  echo "[package] cmake not found, run ./scripts/linux/deps.sh first." >&2
  exit "${EXIT_FAILURE}"
fi

if ! command -v ninja >/dev/null 2>&1; then
  echo "[package] ninja not found, run ./scripts/linux/deps.sh first." >&2
  exit "${EXIT_FAILURE}"
fi

if ! command -v dpkg-deb >/dev/null 2>&1; then
  echo "[package] dpkg-deb not found, install it first." >&2
  exit "${EXIT_FAILURE}"
fi

cmake -S "${PROJECT_ROOT_FOLDER}" \
  -B "${BUILD_FOLDER}" \
  -G Ninja \
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"

cmake --build "${BUILD_FOLDER}"

mkdir -p "${OUTPUT_FOLDER}"
rm -f "${BUILD_FOLDER}/cutegit_"*.deb "${BUILD_FOLDER}/cutegit-"*.tar.gz 2>/dev/null || true
(
  cd "${BUILD_FOLDER}"
  cpack
)

rm -f "${OUTPUT_FOLDER}/cutegit_"*.deb "${OUTPUT_FOLDER}/cutegit-"*.tar.gz 2>/dev/null || true

find "${BUILD_FOLDER}" -maxdepth 1 -type f \
  \( -name "cutegit_${VERSION}_*.deb" -o -name "cutegit-${VERSION}-*.tar.gz" \) \
  -exec cp -f {} "${OUTPUT_FOLDER}/" \;

echo
printf 'Packages generated in %s\n' "${OUTPUT_FOLDER}"
find "${OUTPUT_FOLDER}" -maxdepth 1 -type f \( -name "*.deb" -o -name "*.tar.gz" \) -print | sort
