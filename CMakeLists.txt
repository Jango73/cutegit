cmake_minimum_required(VERSION 3.16)

# Version lives in the VERSION file at the repository root (same file used by
# scripts/linux/package.sh). Fail fast if it is missing or malformed.
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION")
  message(FATAL_ERROR "VERSION file not found at repository root")
endif()
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" CUTEGIT_VERSION_RAW)
string(STRIP "${CUTEGIT_VERSION_RAW}" CUTEGIT_VERSION)
if(NOT CUTEGIT_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
  message(FATAL_ERROR "Malformed VERSION file: '${CUTEGIT_VERSION_RAW}'")
endif()

project(CuteGit VERSION ${CUTEGIT_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS
  Core
  Gui
  Widgets
  Qml
  Quick
  QuickWidgets
  QuickControls2
  QuickTest
  Network
  Xml
  Svg
  Test
)

add_subdirectory(CuteGit)
add_subdirectory(tests)

set(CPACK_PACKAGE_NAME "cutegit")
set(CPACK_PACKAGE_VENDOR "CuteGit")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Graphical versioning client")
set(CPACK_PACKAGE_VERSION "${CUTEGIT_VERSION}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

if(UNIX AND NOT APPLE)
    set(CPACK_GENERATOR "DEB;TGZ")
    set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Jango73 <guillaumed00@gmail.com>")
    set(CPACK_DEBIAN_PACKAGE_SECTION "vcs")
    set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
endif()

include(CPack)
