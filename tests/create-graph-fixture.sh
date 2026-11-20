#!/bin/bash
# Creates the deterministic fixture repository for the graph lane test section.
# Reuses the generic test repository script with a large history so pagination
# and scrolling exercise the ghost commit lanes.
set -euo pipefail

SCRIPT_FOLDER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_FOLDER="$(cd "${SCRIPT_FOLDER}/.." && pwd)"

FIXTURE_FOLDER="${SCRIPT_FOLDER}/fixtures/graph/repo"

"${ROOT_FOLDER}/scripts/linux/create-test-git-repository.sh" \
    --target "${FIXTURE_FOLDER}" \
    --force \
    --branches 4 \
    --merged 4 \
    --base-commits 20 \
    --branch-commits 20 \
    --tags 0 \
    --seed 7 \
    --no-staged \
    --unstaged \
    --no-untracked \
    --no-stash \
    --no-conflict