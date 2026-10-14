#!/bin/bash
# Creates the deterministic fixture repository for the diff test section.
# Reuses the generic test repository script with a minimal layout.
set -euo pipefail

SCRIPT_FOLDER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_FOLDER="$(cd "${SCRIPT_FOLDER}/.." && pwd)"

FIXTURE_FOLDER="${SCRIPT_FOLDER}/fixtures/diff/repo"

"${ROOT_FOLDER}/scripts/linux/create-test-git-repository.sh" \
    --target "${FIXTURE_FOLDER}" \
    --force \
    --branches 0 \
    --merged 0 \
    --base-commits 1 \
    --branch-commits 0 \
    --tags 0 \
    --seed 7 \
    --no-staged \
    --unstaged \
    --no-untracked \
    --no-stash \
    --no-conflict
