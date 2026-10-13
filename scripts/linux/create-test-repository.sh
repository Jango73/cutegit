#!/bin/bash
# Creates a deterministic fictive git repository for manual and graphical testing.
# Default target is ./test, with merged branches, unmerged branches, tags and dirty states.
set -euo pipefail

ROOT_FOLDER="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

EXIT_SUCCESS=0
EXIT_FAILURE=1

TARGET_FOLDER="./test"
BRANCH_COUNT=4
MERGED_COUNT=2
BASE_COMMIT_COUNT=5
BRANCH_COMMIT_COUNT=3
TAG_COUNT=2
BRANCH_PREFIX="feature"
INITIAL_BRANCH="main"
SEED=42
FORCE_ENABLED=1
FORCE_DISABLED=0
FLAG_ENABLED=1
FLAG_DISABLED=0
FORCE_DELETE="${FORCE_DISABLED}"
WITH_STAGED="${FLAG_ENABLED}"
WITH_UNSTAGED="${FLAG_ENABLED}"
WITH_UNTRACKED="${FLAG_ENABLED}"
WITH_STASH="${FLAG_DISABLED}"
WITH_CONFLICT="${FLAG_DISABLED}"

DATE_COUNTER=0
BASE_DATE="2024-01-01 12:00:00 UTC"

###########################################################################
#/**
# * Print usage text on standard output.
# */
usage() {
    cat <<'USAGE'
Usage: ./create-test-repository.sh [TARGET_FOLDER] [options]
    Create a fictive git repository with branches, merges and tags.

Arguments:
    TARGET_FOLDER                 Target folder (default: ./test)

Options:
    --target FOLDER               Same as TARGET_FOLDER argument
    --branches COUNT              Branches created from initial branch (default: 4)
    --merged COUNT                Branches merged back (default: 2, capped to --branches)
    --base-commits COUNT          Commits on initial branch before branching (default: 5)
    --branch-commits COUNT        Commits created on each branch (default: 3)
    --tags COUNT                  Annotated tags created on initial branch (default: 2)
    --branch-prefix PREFIX        Prefix for branch names (default: feature)
    --initial-branch NAME         Initial branch name (default: main)
    --seed SEED                   Seed for deterministic content (default: 42)
    --staged / --no-staged        Leave a staged file (default: staged)
    --unstaged / --no-unstaged    Leave an unstaged modification (default: unstaged)
    --untracked / --no-untracked  Leave an untracked file (default: untracked)
    --stash / --no-stash          Leave a stash entry (default: no stash)
    --conflict / --no-conflict    Leave a merge conflict in progress (default: no conflict)
    --force, -f                   Delete TARGET_FOLDER when it already exists
    -h, --help                    Show this help
USAGE
}

###########################################################################
#/**
# * Print an error message on standard error.
# */
fail() {
    echo "[test-repository] Error: $1" >&2
}

###########################################################################
#/**
# * Return a deterministic date incremented on each call.
# */
next_date() {
    DATE_COUNTER=$((DATE_COUNTER + 1))
    date -u -d "${BASE_DATE} + ${DATE_COUNTER} hours" +"%Y-%m-%dT%H:%M:%S+00:00"
}

###########################################################################
#/**
# * Create one empty commit and one file commit with deterministic content.
# */
make_commit() {
    local message="$1"
    local file="$2"
    local content="$3"
    local commit_date
    commit_date="$(next_date)"
    mkdir -p "$(dirname "${TARGET_FOLDER}/${file}")"
    printf '%s\n' "${content}" >> "${TARGET_FOLDER}/${file}"
    GIT_AUTHOR_DATE="${commit_date}" GIT_COMMITTER_DATE="${commit_date}" \
        git -C "${TARGET_FOLDER}" add "${file}"
    GIT_AUTHOR_DATE="${commit_date}" GIT_COMMITTER_DATE="${commit_date}" \
        git -C "${TARGET_FOLDER}" commit -q -m "${message}"
}

###########################################################################
#/**
# * Read a non negative integer option value.
# */
read_count() {
    local name="$1"
    local value="${2:-}"
    if [[ ! "${value}" =~ ^[0-9]+$ ]]; then
        fail "Option ${name} requires a non negative integer, got '${value}'."
        exit "${EXIT_FAILURE}"
    fi
    printf '%s' "${value}"
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        -h|--help)
            usage
            exit "${EXIT_SUCCESS}"
            ;;
        --target)
            TARGET_FOLDER="${2:-}"
            shift 2
            ;;
        --branches)
            BRANCH_COUNT="$(read_count "$1" "${2:-}")"
            shift 2
            ;;
        --merged)
            MERGED_COUNT="$(read_count "$1" "${2:-}")"
            shift 2
            ;;
        --base-commits)
            BASE_COMMIT_COUNT="$(read_count "$1" "${2:-}")"
            shift 2
            ;;
        --branch-commits)
            BRANCH_COMMIT_COUNT="$(read_count "$1" "${2:-}")"
            shift 2
            ;;
        --tags)
            TAG_COUNT="$(read_count "$1" "${2:-}")"
            shift 2
            ;;
        --branch-prefix)
            BRANCH_PREFIX="${2:-}"
            shift 2
            ;;
        --initial-branch)
            INITIAL_BRANCH="${2:-}"
            shift 2
            ;;
        --seed)
            SEED="${2:-}"
            shift 2
            ;;
        --staged)
            WITH_STAGED="${FLAG_ENABLED}"
            shift
            ;;
        --no-staged)
            WITH_STAGED="${FLAG_DISABLED}"
            shift
            ;;
        --unstaged)
            WITH_UNSTAGED="${FLAG_ENABLED}"
            shift
            ;;
        --no-unstaged)
            WITH_UNSTAGED="${FLAG_DISABLED}"
            shift
            ;;
        --untracked)
            WITH_UNTRACKED="${FLAG_ENABLED}"
            shift
            ;;
        --no-untracked)
            WITH_UNTRACKED="${FLAG_DISABLED}"
            shift
            ;;
        --stash)
            WITH_STASH="${FLAG_ENABLED}"
            shift
            ;;
        --no-stash)
            WITH_STASH="${FLAG_DISABLED}"
            shift
            ;;
        --conflict)
            WITH_CONFLICT="${FLAG_ENABLED}"
            shift
            ;;
        --no-conflict)
            WITH_CONFLICT="${FLAG_DISABLED}"
            shift
            ;;
        --force|-f)
            FORCE_DELETE="${FORCE_ENABLED}"
            shift
            ;;
        --*)
            fail "Unknown option: $1"
            usage >&2
            exit "${EXIT_FAILURE}"
            ;;
        *)
            TARGET_FOLDER="$1"
            shift
            ;;
    esac
done

if [[ -z "${TARGET_FOLDER}" ]]; then
    fail "Target folder must not be empty."
    exit "${EXIT_FAILURE}"
fi

if [[ "${MERGED_COUNT}" -gt "${BRANCH_COUNT}" ]]; then
    echo "[test-repository] Merged count capped to branch count." >&2
    MERGED_COUNT="${BRANCH_COUNT}"
fi

if [[ -e "${TARGET_FOLDER}" ]]; then
    if [[ "${FORCE_DELETE}" -eq "${FORCE_ENABLED}" ]]; then
        rm -rf "${TARGET_FOLDER}"
    else
        fail "Target '${TARGET_FOLDER}' already exists, use --force to replace it."
        exit "${EXIT_FAILURE}"
    fi
fi

mkdir -p "${TARGET_FOLDER}"

if git init -b "${INITIAL_BRANCH}" -q "${TARGET_FOLDER}" 2>/dev/null; then
    true
else
    git init -q "${TARGET_FOLDER}"
    git -C "${TARGET_FOLDER}" checkout -q -b "${INITIAL_BRANCH}"
fi

git -C "${TARGET_FOLDER}" config user.name "Test User"
git -C "${TARGET_FOLDER}" config user.email "test.user@example.com"
git -C "${TARGET_FOLDER}" config commit.gpgsign false

RANDOM="${SEED}"

index=1
while [[ "${index}" -le "${BASE_COMMIT_COUNT}" ]]; do
    make_commit "Base commit ${index} on ${INITIAL_BRANCH}" "source/module-${SEED}.txt" "Base line ${index} seed ${SEED} value ${RANDOM}"
    index=$((index + 1))
done

index=1
while [[ "${index}" -le "${BRANCH_COUNT}" ]]; do
    branch_name="${BRANCH_PREFIX}-$(printf '%02d' "${index}")"
    git -C "${TARGET_FOLDER}" checkout -q -b "${branch_name}" "${INITIAL_BRANCH}"
    commit=1
    while [[ "${commit}" -le "${BRANCH_COMMIT_COUNT}" ]]; do
        make_commit "Commit ${commit} on ${branch_name}" "source/${branch_name}.txt" "Branch ${branch_name} line ${commit} value ${RANDOM}"
        commit=$((commit + 1))
    done
    git -C "${TARGET_FOLDER}" checkout -q "${INITIAL_BRANCH}"
    index=$((index + 1))
done

index=1
while [[ "${index}" -le "${MERGED_COUNT}" ]]; do
    branch_name="${BRANCH_PREFIX}-$(printf '%02d' "${index}")"
    merge_date="$(next_date)"
    GIT_AUTHOR_DATE="${merge_date}" GIT_COMMITTER_DATE="${merge_date}" \
        git -C "${TARGET_FOLDER}" merge -q --no-ff -m "Merge branch '${branch_name}'" "${branch_name}"
    index=$((index + 1))
done

index=1
while [[ "${index}" -le "${TAG_COUNT}" ]]; do
    tag_date="$(next_date)"
    GIT_AUTHOR_DATE="${tag_date}" GIT_COMMITTER_DATE="${tag_date}" \
        git -C "${TARGET_FOLDER}" tag -a "version-0.${index}.0" -m "Release 0.${index}.0" "${INITIAL_BRANCH}~$((TAG_COUNT - index))"
    index=$((index + 1))
done

if [[ "${WITH_STAGED}" -eq "${FLAG_ENABLED}" ]]; then
    printf 'Staged content seed %s\n' "${SEED}" > "${TARGET_FOLDER}/staged-example.txt"
    git -C "${TARGET_FOLDER}" add staged-example.txt
fi

if [[ "${WITH_UNSTAGED}" -eq "${FLAG_ENABLED}" ]]; then
    printf 'Unstaged modification value %s\n' "${RANDOM}" >> "${TARGET_FOLDER}/source/module-${SEED}.txt"
fi

if [[ "${WITH_UNTRACKED}" -eq "${FLAG_ENABLED}" ]]; then
    printf 'Untracked notes seed %s\n' "${SEED}" > "${TARGET_FOLDER}/notes-untracked.txt"
fi

if [[ "${WITH_STASH}" -eq "${FLAG_ENABLED}" ]]; then
    printf 'Stashed content value %s\n' "${RANDOM}" > "${TARGET_FOLDER}/stashed-example.txt"
    git -C "${TARGET_FOLDER}" add stashed-example.txt
    git -C "${TARGET_FOLDER}" stash push -q -m "Example stash"
fi

if [[ "${WITH_CONFLICT}" -eq "${FLAG_ENABLED}" ]]; then
    git -C "${TARGET_FOLDER}" checkout -q -b "${BRANCH_PREFIX}-conflict-a" "${INITIAL_BRANCH}"
    printf 'Conflict line from A\n' > "${TARGET_FOLDER}/conflict.txt"
    git -C "${TARGET_FOLDER}" add conflict.txt
    git -C "${TARGET_FOLDER}" commit -q -m "Conflict side A"
    git -C "${TARGET_FOLDER}" checkout -q -b "${BRANCH_PREFIX}-conflict-b" "${INITIAL_BRANCH}"
    printf 'Conflict line from B\n' > "${TARGET_FOLDER}/conflict.txt"
    git -C "${TARGET_FOLDER}" add conflict.txt
    git -C "${TARGET_FOLDER}" commit -q -m "Conflict side B"
    git -C "${TARGET_FOLDER}" checkout -q "${BRANCH_PREFIX}-conflict-a"
    if git -C "${TARGET_FOLDER}" merge "${BRANCH_PREFIX}-conflict-b"; then
        echo "[test-repository] Conflict setup did not conflict." >&2
    fi
fi

echo "[test-repository] Created at ${TARGET_FOLDER}"
git -C "${TARGET_FOLDER}" branch --list
git -C "${TARGET_FOLDER}" log --graph --oneline --decorate --all -n 20
git -C "${TARGET_FOLDER}" status --short --branch
