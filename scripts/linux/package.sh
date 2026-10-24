#!/bin/bash
set -euo pipefail

SCRIPT_FOLDER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT_FOLDER="$(cd "${SCRIPT_FOLDER}/../.." && pwd)"
BUILD_TYPE="Release"
BUILD_FOLDER="${PROJECT_ROOT_FOLDER}/build-package-${BUILD_TYPE,,}"
OUTPUT_FOLDER="${PROJECT_ROOT_FOLDER}/dist"
VERSION="$(tr -d '[:space:]' < "${PROJECT_ROOT_FOLDER}/VERSION")"

EXIT_SUCCESS=0
EXIT_FAILURE=1

echo "Building CuteGit ${VERSION}..."

# Build the project (populates build-release via scripts/linux/build.sh)
"${SCRIPT_FOLDER}/build.sh" -c "${BUILD_TYPE}"

SOURCE_BUILD_FOLDER="${PROJECT_ROOT_FOLDER}/build-${BUILD_TYPE,,}"

# Determine executable name
EXECUTABLE_NAME="CuteGitApp"
EXECUTABLE_PATH="${SOURCE_BUILD_FOLDER}/CuteGit/bin/${EXECUTABLE_NAME}"

if [[ ! -f "${EXECUTABLE_PATH}" ]]; then
  echo "Error: Executable not found at ${EXECUTABLE_PATH}" >&2
  exit "${EXIT_FAILURE}"
fi

# Create distribution directory
rm -rf "${BUILD_FOLDER}" "${OUTPUT_FOLDER}"
mkdir -p "${BUILD_FOLDER}" "${OUTPUT_FOLDER}"

# Create package directory structure (staged in isolated build folder)
PKG_NAME="CuteGit-${VERSION}-linux-x64"
PKG_DIR="${BUILD_FOLDER}/${PKG_NAME}"
mkdir -p "${PKG_DIR}"

# Copy executable
cp "${EXECUTABLE_PATH}" "${PKG_DIR}/"

# Copy descriptive UI layouts next to the binary
if [[ -f "${PROJECT_ROOT_FOLDER}/CuteGit/deploy/layouts.json" ]]; then
  cp "${PROJECT_ROOT_FOLDER}/CuteGit/deploy/layouts.json" "${PKG_DIR}/"
fi

# Copy deploy files if they exist
if [[ -d "${PROJECT_ROOT_FOLDER}/CuteGit/deploy/linux" ]]; then
  cp -r "${PROJECT_ROOT_FOLDER}/CuteGit/deploy/linux/"* "${PKG_DIR}/" 2>/dev/null || true
fi

# Create a simple run script
cat > "${PKG_DIR}/run.sh" << 'EOF'
#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export LD_LIBRARY_PATH="${SCRIPT_DIR}:${LD_LIBRARY_PATH}"
cd "${SCRIPT_DIR}"
exec ./CuteGitApp "$@"
EOF
chmod +x "${PKG_DIR}/run.sh"

# Copy LICENSE
cp "${PROJECT_ROOT_FOLDER}/LICENSE" "${PKG_DIR}/"

# Create archive in dist/
tar -czf "${OUTPUT_FOLDER}/${PKG_NAME}.tar.gz" -C "${BUILD_FOLDER}" "${PKG_NAME}"

# Clean up temporary staging directory
rm -rf "${PKG_DIR}"

echo ""
echo "Package created: ${OUTPUT_FOLDER}/${PKG_NAME}.tar.gz"
echo "To extract and run:"
echo "  tar -xzf ${PKG_NAME}.tar.gz"
echo "  cd ${PKG_NAME}"
echo "  ./run.sh"
