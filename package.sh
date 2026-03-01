#!/bin/bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_TYPE="Release"
BUILD_DIR="${ROOT_DIR}/build-${BUILD_TYPE,,}"
DIST_DIR="${ROOT_DIR}/dist"
VERSION="1.0.1"  # Default version, can be overridden by VERSION file

if [[ -f "${ROOT_DIR}/VERSION" ]]; then
    VERSION="$(tr -d '[:space:]' < "${ROOT_DIR}/VERSION")"
fi

echo "Building CuteGit ${VERSION}..."

# Build the project
"${ROOT_DIR}/build.sh" --release

# Create distribution directory
rm -rf "${DIST_DIR}"
mkdir -p "${DIST_DIR}"

# Determine executable name
EXECUTABLE_NAME="CuteGitApp"
EXECUTABLE_PATH="${BUILD_DIR}/bin/${EXECUTABLE_NAME}"

if [[ ! -f "${EXECUTABLE_PATH}" ]]; then
    echo "Error: Executable not found at ${EXECUTABLE_PATH}" >&2
    exit 1
fi

# Create package directory structure
PKG_NAME="CuteGit-${VERSION}-linux-x64"
PKG_DIR="${DIST_DIR}/${PKG_NAME}"
mkdir -p "${PKG_DIR}"

# Copy executable
cp "${EXECUTABLE_PATH}" "${PKG_DIR}/"

# Copy deploy files if they exist
if [[ -d "${ROOT_DIR}/CuteGit/deploy/linux" ]]; then
    cp -r "${ROOT_DIR}/CuteGit/deploy/linux/"* "${PKG_DIR}/" 2>/dev/null || true
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
cp "${ROOT_DIR}/LICENSE" "${PKG_DIR}/"

# Create archive
cd "${DIST_DIR}"
tar -czf "${PKG_NAME}.tar.gz" "${PKG_NAME}"

# Clean up temporary directory
rm -rf "${PKG_DIR}"

echo ""
echo "Package created: ${DIST_DIR}/${PKG_NAME}.tar.gz"
echo "To extract and run:"
echo "  tar -xzf ${PKG_NAME}.tar.gz"
echo "  cd ${PKG_NAME}"
echo "  ./run.sh"