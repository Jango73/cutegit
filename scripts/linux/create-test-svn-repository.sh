#!/bin/bash
# Creates a deterministic fictive Subversion repository for manual and graphical testing.
# Default target is ./test-svn (working copy of trunk), backed by ./test-svn-repo.
# Layout is trunk/branches/tags, with merged branches, tags and dirty states.
# Dirty state mapping: --staged leaves a scheduled add (A), --unstaged a
# modification (M), --untracked an unversioned file (?), --stash a shelf entry
# (svn shelve), --conflict a merge conflict in progress (C).
set -euo pipefail

EXIT_SUCCESS=0
EXIT_FAILURE=1

TARGET_FOLDER="./test-svn"
REPO_FOLDER=""
BRANCH_COUNT=4
MERGED_COUNT=2
BASE_COMMIT_COUNT=5
BRANCH_COMMIT_COUNT=3
TAG_COUNT=2
BRANCH_PREFIX="feature"
SEED=42
SVN_USER="test.user"
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
Usage: ./create-test-svn-repository.sh [TARGET_FOLDER] [options]
    Create a fictive Subversion repository with branches, merges and tags.

Arguments:
    TARGET_FOLDER                 Working copy of trunk (default: ./test-svn)

Options:
    --target FOLDER               Same as TARGET_FOLDER argument
    --repo FOLDER                 Repository database folder (default: <TARGET_FOLDER>-repo)
    --branches COUNT              Branches created from trunk (default: 4)
    --merged COUNT                Branches merged back into trunk (default: 2, capped to --branches)
    --base-commits COUNT          Commits on trunk before branching (default: 5)
    --branch-commits COUNT        Commits created on each branch (default: 3)
    --tags COUNT                  Tags copied from trunk (default: 2)
    --branch-prefix PREFIX        Prefix for branch names (default: feature)
    --seed SEED                   Seed for deterministic content (default: 42)
    --staged / --no-staged        Leave a scheduled add (default: staged)
    --unstaged / --no-unstaged    Leave a modification (default: unstaged)
    --untracked / --no-untracked  Leave an unversioned file (default: untracked)
    --stash / --no-stash          Leave a shelf entry via svn shelve (default: no stash)
    --conflict / --no-conflict    Leave a merge conflict in progress (default: no conflict)
    --force, -f                   Delete TARGET_FOLDER and REPO_FOLDER when they already exist
    -h, --help                    Show this help
USAGE
}

###########################################################################
#/**
# * Print an error message on standard error.
# */
fail() {
    echo "[test-svn-repository] Error: $1" >&2
}

###########################################################################
#/**
# * Check that a required command is available.
# */
require_command() {
    if ! command -v "$1" >/dev/null 2>&1; then
        fail "Required command '$1' not found, install the subversion package."
        exit "${EXIT_FAILURE}"
    fi
}

###########################################################################
#/**
# * Return a deterministic date incremented on each call.
# */
next_date() {
    DATE_COUNTER=$((DATE_COUNTER + 1))
    date -u -d "${BASE_DATE} + ${DATE_COUNTER} hours" +"%Y-%m-%dT%H:%M:%S.000000Z"
}

###########################################################################
#/**
# * Set the svn:date revision property of a repository revision.
# */
set_revision_date() {
    local revision="$1"
    local revision_date
    revision_date="$(next_date)"
    svn propset -q --revprop -r "${revision}" svn:date "${revision_date}" "${REPO_URL}" --username "${SVN_USER}" --non-interactive
}

###########################################################################
#/**
# * Return the youngest revision of the repository.
# */
head_revision() {
    svn info --show-item revision "${REPO_URL}"
}

###########################################################################
#/**
# * Commit one file change on trunk with deterministic content.
# */
make_trunk_commit() {
    local message="$1"
    local file="$2"
    local content="$3"
    mkdir -p "$(dirname "${TARGET_FOLDER}/${file}")"
    printf '%s\n' "${content}" >> "${TARGET_FOLDER}/${file}"
    svn add -q --parents "${TARGET_FOLDER}/${file}" 2>/dev/null || true
    svn commit -q -m "${message}" --username "${SVN_USER}" --non-interactive "${TARGET_FOLDER}"
    set_revision_date "$(head_revision)"
}

###########################################################################
#/**
# * Commit one file change in a branch working copy with deterministic content.
# */
make_branch_commit() {
    local branch_checkout="$1"
    local message="$2"
    local file="$3"
    local content="$4"
    mkdir -p "$(dirname "${branch_checkout}/${file}")"
    printf '%s\n' "${content}" >> "${branch_checkout}/${file}"
    svn add -q --parents "${branch_checkout}/${file}" 2>/dev/null || true
    svn commit -q -m "${message}" --username "${SVN_USER}" --non-interactive "${branch_checkout}"
    set_revision_date "$(head_revision)"
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
        --repo)
            REPO_FOLDER="${2:-}"
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

if [[ -z "${REPO_FOLDER}" ]]; then
    REPO_FOLDER="${TARGET_FOLDER}-repo"
fi

if [[ "${MERGED_COUNT}" -gt "${BRANCH_COUNT}" ]]; then
    echo "[test-svn-repository] Merged count capped to branch count." >&2
    MERGED_COUNT="${BRANCH_COUNT}"
fi

require_command svn
require_command svnadmin

for existing_folder in "${TARGET_FOLDER}" "${REPO_FOLDER}"; do
    if [[ -e "${existing_folder}" ]]; then
        if [[ "${FORCE_DELETE}" -eq "${FORCE_ENABLED}" ]]; then
            rm -rf "${existing_folder}"
        else
            fail "Target '${existing_folder}' already exists, use --force to replace it."
            exit "${EXIT_FAILURE}"
        fi
    fi
done

REPO_PARENT="$(dirname "${REPO_FOLDER}")"
mkdir -p "${REPO_PARENT}"
svnadmin create "${REPO_FOLDER}"
printf '#!/bin/sh\nexit 0\n' > "${REPO_FOLDER}/hooks/pre-revprop-change"
chmod +x "${REPO_FOLDER}/hooks/pre-revprop-change"

REPO_ABSOLUTE="$(cd "${REPO_FOLDER}" && pwd)"
REPO_URL="file://${REPO_ABSOLUTE}"

svn mkdir -q -m "Create standard layout" --username "${SVN_USER}" --non-interactive \
    "${REPO_URL}/trunk" "${REPO_URL}/branches" "${REPO_URL}/tags"
set_revision_date "$(head_revision)"

svn checkout -q "${REPO_URL}/trunk" "${TARGET_FOLDER}"

RANDOM="${SEED}"

index=1
while [[ "${index}" -le "${BASE_COMMIT_COUNT}" ]]; do
    make_trunk_commit "Base commit ${index} on trunk" "source/module-${SEED}.txt" "Base line ${index} seed ${SEED} value ${RANDOM}"
    index=$((index + 1))
done

SCRATCH_FOLDER="$(mktemp -d)"
trap 'rm -rf "${SCRATCH_FOLDER}"' EXIT

index=1
while [[ "${index}" -le "${BRANCH_COUNT}" ]]; do
    branch_name="${BRANCH_PREFIX}-$(printf '%02d' "${index}")"
    svn copy -q -m "Create branch ${branch_name}" --username "${SVN_USER}" --non-interactive \
        "${REPO_URL}/trunk" "${REPO_URL}/branches/${branch_name}"
    set_revision_date "$(head_revision)"
    branch_checkout="${SCRATCH_FOLDER}/${branch_name}"
    svn checkout -q "${REPO_URL}/branches/${branch_name}" "${branch_checkout}"
    commit=1
    while [[ "${commit}" -le "${BRANCH_COMMIT_COUNT}" ]]; do
        make_branch_commit "${branch_checkout}" "Commit ${commit} on ${branch_name}" "source/${branch_name}.txt" "Branch ${branch_name} line ${commit} value ${RANDOM}"
        commit=$((commit + 1))
    done
    index=$((index + 1))
done

svn update -q "${TARGET_FOLDER}"

index=1
while [[ "${index}" -le "${MERGED_COUNT}" ]]; do
    branch_name="${BRANCH_PREFIX}-$(printf '%02d' "${index}")"
    svn update -q "${TARGET_FOLDER}"
    (cd "${TARGET_FOLDER}" && svn merge -q "^/branches/${branch_name}" .)
    svn commit -q -m "Merge branch '${branch_name}'" --username "${SVN_USER}" --non-interactive "${TARGET_FOLDER}"
    set_revision_date "$(head_revision)"
    index=$((index + 1))
done

HEAD_REVISION="$(head_revision)"
index=1
while [[ "${index}" -le "${TAG_COUNT}" ]]; do
    tag_name="version-0.${index}.0"
    tag_source_revision=$((HEAD_REVISION - TAG_COUNT + index))
    if [[ "${tag_source_revision}" -ge 1 ]]; then
        svn copy -q -r "${tag_source_revision}" -m "Release 0.${index}.0" --username "${SVN_USER}" --non-interactive \
            "${REPO_URL}/trunk" "${REPO_URL}/tags/${tag_name}"
    else
        svn copy -q -m "Release 0.${index}.0" --username "${SVN_USER}" --non-interactive \
            "${REPO_URL}/trunk" "${REPO_URL}/tags/${tag_name}"
    fi
    set_revision_date "$(head_revision)"
    index=$((index + 1))
done

svn update -q "${TARGET_FOLDER}"

if [[ "${WITH_STASH}" -eq "${FLAG_ENABLED}" ]]; then
    printf 'Stashed content value %s\n' "${RANDOM}" > "${TARGET_FOLDER}/stashed-example.txt"
    svn add -q --parents "${TARGET_FOLDER}/stashed-example.txt"
    if svn help shelve >/dev/null 2>&1 \
        && ((cd "${TARGET_FOLDER}" && svn shelve -q "example-shelf" "stashed-example.txt") \
            || (cd "${TARGET_FOLDER}" && svn shelve -q --name "example-shelf" "stashed-example.txt")); then
        true
    else
        echo "[test-svn-repository] svn shelve unavailable, leaving stashed-example.txt scheduled." >&2
    fi
fi

if [[ "${WITH_STAGED}" -eq "${FLAG_ENABLED}" ]]; then
    printf 'Staged content seed %s\n' "${SEED}" > "${TARGET_FOLDER}/staged-example.txt"
    svn add -q --parents "${TARGET_FOLDER}/staged-example.txt"
fi

if [[ "${WITH_UNSTAGED}" -eq "${FLAG_ENABLED}" ]]; then
    printf 'Unstaged modification value %s\n' "${RANDOM}" >> "${TARGET_FOLDER}/source/module-${SEED}.txt"
fi

if [[ "${WITH_UNTRACKED}" -eq "${FLAG_ENABLED}" ]]; then
    printf 'Untracked notes seed %s\n' "${SEED}" > "${TARGET_FOLDER}/notes-untracked.txt"
fi

if [[ "${WITH_CONFLICT}" -eq "${FLAG_ENABLED}" ]]; then
    printf 'Base conflict line\n' > "${TARGET_FOLDER}/conflict.txt"
    svn add -q --parents "${TARGET_FOLDER}/conflict.txt" 2>/dev/null || true
    svn commit -q -m "Base conflict content" --username "${SVN_USER}" --non-interactive "${TARGET_FOLDER}/conflict.txt"
    set_revision_date "$(head_revision)"
    svn copy -q -m "Create branch ${BRANCH_PREFIX}-conflict-b" --username "${SVN_USER}" --non-interactive \
        "${REPO_URL}/trunk" "${REPO_URL}/branches/${BRANCH_PREFIX}-conflict-b"
    set_revision_date "$(head_revision)"
    conflict_checkout="${SCRATCH_FOLDER}/${BRANCH_PREFIX}-conflict-b"
    svn checkout -q "${REPO_URL}/branches/${BRANCH_PREFIX}-conflict-b" "${conflict_checkout}"
    printf 'Conflict line from B\n' > "${conflict_checkout}/conflict.txt"
    svn commit -q -m "Conflict side B" --username "${SVN_USER}" --non-interactive "${conflict_checkout}"
    set_revision_date "$(head_revision)"
    printf 'Conflict line from A\n' > "${TARGET_FOLDER}/conflict.txt"
    svn commit -q -m "Conflict side A" --username "${SVN_USER}" --non-interactive "${TARGET_FOLDER}/conflict.txt"
    set_revision_date "$(head_revision)"
    svn update -q "${TARGET_FOLDER}"
    (cd "${TARGET_FOLDER}" && svn merge -q "^/branches/${BRANCH_PREFIX}-conflict-b" .) || true
    if ! (cd "${TARGET_FOLDER}" && svn status | grep -q '^C'); then
        echo "[test-svn-repository] Conflict setup did not conflict." >&2
    fi
fi

echo "[test-svn-repository] Working copy at ${TARGET_FOLDER}, repository at ${REPO_FOLDER}"
svn info "${TARGET_FOLDER}" | grep -E '^(URL|Relative URL|Revision):'
svn list "${REPO_URL}/branches"
svn list "${REPO_URL}/tags"
(cd "${TARGET_FOLDER}" && svn log --limit 20)
(cd "${TARGET_FOLDER}" && svn status)
