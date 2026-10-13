#!/bin/bash
# Install (or uninstall) a GNOME/Pop!_OS application shortcut (.desktop file)
# pointing at the local build. Defaults to the Release binary.
set -euo pipefail

SCRIPT_FOLDER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_FOLDER="$(cd "${SCRIPT_FOLDER}/../.." && pwd)"

EXIT_SUCCESS=0
EXIT_FAILURE=1

BUILD_TYPE="Release"
APP_NAME="CuteGit"
DESKTOP_NAME="cutegit"
ICON_PATH=""
NO_ICON=0
UNINSTALL=0

usage() {
  cat <<'USAGE'
Usage: ./install-desktop.sh [--release|--debug] [--name NAME] [--icon PATH] [--no-icon] [--uninstall]
  --release, -r  Point at the Release binary (default)
  --debug,   -d  Point at the Debug binary
  -c             Build type (Debug or Release)
  --name         Desktop file base name and app name suffix (default: cutegit)
  --icon         Absolute path to an icon file (default: Media/cutegit-icon.svg)
  --no-icon      Omit the Icon entry
  --uninstall    Remove the installed shortcut instead of creating it
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
    -c)
      if [[ -z "${2:-}" ]]; then
        echo "Option -c requires an argument." >&2
        usage >&2
        exit "${EXIT_FAILURE}"
      fi
      BUILD_TYPE="$2"
      shift 2
      ;;
    --name)
      if [[ -z "${2:-}" ]]; then
        echo "Option --name requires an argument." >&2
        usage >&2
        exit "${EXIT_FAILURE}"
      fi
      DESKTOP_NAME="$2"
      shift 2
      ;;
    --icon)
      if [[ -z "${2:-}" ]]; then
        echo "Option --icon requires an argument." >&2
        usage >&2
        exit "${EXIT_FAILURE}"
      fi
      ICON_PATH="$2"
      shift 2
      ;;
    --no-icon)
      NO_ICON=1
      shift
      ;;
    --uninstall)
      UNINSTALL=1
      shift
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

DESKTOP_FILE="${HOME}/.local/share/applications/${DESKTOP_NAME}.desktop"

if [[ "${UNINSTALL}" == "1" ]]; then
  rm -f "${DESKTOP_FILE}"
  if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database "${HOME}/.local/share/applications" >/dev/null 2>&1 || true
  fi
  echo "[install-desktop] Removed ${DESKTOP_FILE}"
  exit "${EXIT_SUCCESS}"
fi

if [[ -n "${ICON_PATH}" && ! -f "${ICON_PATH}" ]]; then
  echo "[install-desktop] Icon not found: ${ICON_PATH}" >&2
  exit "${EXIT_FAILURE}"
fi

if [[ "${NO_ICON}" == "1" ]]; then
  ICON_PATH=""
elif [[ -z "${ICON_PATH}" ]]; then
  DEFAULT_ICON="${ROOT_FOLDER}/Media/cutegit-icon.svg"
  if [[ -f "${DEFAULT_ICON}" ]]; then
    ICON_PATH="${DEFAULT_ICON}"
  fi
fi

BUILD_FOLDER="${ROOT_FOLDER}/build-${BUILD_TYPE,,}"

if [[ "${BUILD_TYPE}" == "Debug" ]]; then
  EXECUTABLE_NAME="CuteGitApp-dbg"
  DISPLAY_NAME="${APP_NAME} (Debug)"
else
  EXECUTABLE_NAME="CuteGitApp"
  DISPLAY_NAME="${APP_NAME}"
fi

EXECUTABLE_PATH="${BUILD_FOLDER}/CuteGit/bin/${EXECUTABLE_NAME}"

if [[ ! -f "${EXECUTABLE_PATH}" ]]; then
  echo "[install-desktop] Executable not found: ${EXECUTABLE_PATH}" >&2
  echo "[install-desktop] Run ./scripts/linux/build.sh first." >&2
  exit "${EXIT_FAILURE}"
fi

mkdir -p "$(dirname "${DESKTOP_FILE}")"

{
  echo "[Desktop Entry]"
  echo "Type=Application"
  echo "Version=1.0"
  echo "Name=${DISPLAY_NAME}"
  echo "Comment=Graphical Git client"
  echo "Exec=/usr/bin/env LD_LIBRARY_PATH=${ROOT_FOLDER}/qt-plus/bin ${EXECUTABLE_PATH}"
  echo "Path=${ROOT_FOLDER}"
  if [[ -n "${ICON_PATH}" ]]; then
    echo "Icon=${ICON_PATH}"
  fi
  echo "Terminal=false"
  echo "Categories=Development;RevisionControl;"
} > "${DESKTOP_FILE}"

chmod 644 "${DESKTOP_FILE}"

if command -v update-desktop-database >/dev/null 2>&1; then
  update-desktop-database "${HOME}/.local/share/applications" >/dev/null 2>&1 || true
fi

if command -v desktop-file-validate >/dev/null 2>&1; then
  if ! desktop-file-validate "${DESKTOP_FILE}"; then
    echo "[install-desktop] Warning: ${DESKTOP_FILE} failed validation." >&2
  fi
fi

echo "[install-desktop] Installed ${DESKTOP_FILE}"
