#!/bin/bash
set -euo pipefail

SCRIPT_FOLDER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT_FOLDER="$(cd "${SCRIPT_FOLDER}/../.." && pwd)"
VERSION_FILE="${PROJECT_ROOT_FOLDER}/VERSION"

usage() {
  cat <<'USAGE'
Usage: ./set-version.sh <version>
Example: ./set-version.sh 1.2.3
USAGE
}

if [[ $# -ne 1 ]]; then
  usage >&2
  exit 1
fi

NEW_VERSION="$1"
if [[ ! "${NEW_VERSION}" =~ ^[0-9]+\.[0-9]+\.[0-9]+([.-][0-9A-Za-z.-]+)?$ ]]; then
  echo "Invalid version: ${NEW_VERSION}" >&2
  echo "Expected format: MAJOR.MINOR.PATCH (optional suffix allowed)" >&2
  exit 1
fi

printf '%s\n' "${NEW_VERSION}" > "${VERSION_FILE}"

# CMakeLists.txt files read the VERSION file at configure time, so no other
# file needs editing when the version changes.

printf 'Version set to %s\n' "${NEW_VERSION}"
