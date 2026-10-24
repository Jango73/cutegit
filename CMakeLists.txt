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
