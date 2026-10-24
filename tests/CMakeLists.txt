# Test binaries. They compile the CuteGit sources they need directly (plus
# CXMLNode.cpp from qt-plus), exactly like the former tests/*.pro files did.
# Binaries land in tests/bin/, the folder scripts/linux/ci-local.sh runs.
#
# This folder also configures standalone (ci-local.sh runs cmake -S tests/),
# so the standalone block below duplicates the matching root settings.

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  cmake_minimum_required(VERSION 3.16)
  file(READ "${CMAKE_CURRENT_SOURCE_DIR}/../VERSION" CUTEGIT_VERSION_RAW)
  string(STRIP "${CUTEGIT_VERSION_RAW}" CUTEGIT_VERSION)
  project(CuteGitTests VERSION ${CUTEGIT_VERSION} LANGUAGES CXX)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
  set(CMAKE_AUTOMOC ON)
  set(CMAKE_AUTORCC ON)
  find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Qml Quick QuickControls2 QuickTest Test Xml)
endif()

set(CUTEGIT_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../CuteGit/sources")
set(QTPLUS_CPP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../qt-plus/source/cpp")

set(PATCHBUILDER_HEADERS
  "${CUTEGIT_SOURCES_DIR}/CPatchBuilder.h"
  "${CUTEGIT_SOURCES_DIR}/CDiffLine.h"
  "${CUTEGIT_SOURCES_DIR}/CDiffConstants.h"
  "${CUTEGIT_SOURCES_DIR}/CDiffModel.h"
  "${CUTEGIT_SOURCES_DIR}/CDiffModelProxy.h"
  "${CUTEGIT_SOURCES_DIR}/CEnums.h"
)

set(PATCHBUILDER_SOURCES
  "${CUTEGIT_SOURCES_DIR}/CPatchBuilder.cpp"
  "${CUTEGIT_SOURCES_DIR}/CDiffConstants.cpp"
  "${CUTEGIT_SOURCES_DIR}/CDiffModel.cpp"
  "${CUTEGIT_SOURCES_DIR}/CDiffModelProxy.cpp"
)

set(REPOSITORY_SOURCES
  "${CUTEGIT_SOURCES_DIR}/CRepository.cpp"
  "${CUTEGIT_SOURCES_DIR}/CRepositoryDiff.cpp"
  "${CUTEGIT_SOURCES_DIR}/CRepositoryOutput.cpp"
  ${PATCHBUILDER_SOURCES}
  "${CUTEGIT_SOURCES_DIR}/CFlatFileModel.cpp"
  "${CUTEGIT_SOURCES_DIR}/CFlatFileModelProxy.cpp"
  "${CUTEGIT_SOURCES_DIR}/CStagedFileModelProxy.cpp"
  "${CUTEGIT_SOURCES_DIR}/CBranchModel.cpp"
  "${CUTEGIT_SOURCES_DIR}/CCommitLogModel.cpp"
  "${CUTEGIT_SOURCES_DIR}/CLogModel.cpp"
  "${CUTEGIT_SOURCES_DIR}/CLogModelProxy.cpp"
  "${CUTEGIT_SOURCES_DIR}/CLogLineCollection.cpp"
  "${CUTEGIT_SOURCES_DIR}/CGraphModel.cpp"
  "${CUTEGIT_SOURCES_DIR}/CGraphLine.cpp"
  "${CUTEGIT_SOURCES_DIR}/CGraphLineCollection.cpp"
  "${CUTEGIT_SOURCES_DIR}/CGraphLayout.cpp"
  "${CUTEGIT_SOURCES_DIR}/CRepoFile.cpp"
  "${CUTEGIT_SOURCES_DIR}/CLabelModel.cpp"
  "${CUTEGIT_SOURCES_DIR}/CUtils.cpp"
  "${CUTEGIT_SOURCES_DIR}/Strings.cpp"
  "${CUTEGIT_SOURCES_DIR}/commands/CCommands.cpp"
  "${CUTEGIT_SOURCES_DIR}/commands/CGitCommands.cpp"
  "${CUTEGIT_SOURCES_DIR}/commands/CGerritCommands.cpp"
  "${CUTEGIT_SOURCES_DIR}/commands/CSvnCommands.cpp"
  "${CUTEGIT_SOURCES_DIR}/commands/CHgCommands.cpp"
  "${CUTEGIT_SOURCES_DIR}/commands/CExecution.cpp"
  "${QTPLUS_CPP_DIR}/CXMLNode.cpp"
)

set(REPOSITORY_HEADERS
  "${CUTEGIT_SOURCES_DIR}/Strings.h"
  "${CUTEGIT_SOURCES_DIR}/CRepository.h"
  "${CUTEGIT_SOURCES_DIR}/CPatchBuilder.h"
  "${CUTEGIT_SOURCES_DIR}/CDiffConstants.h"
  "${CUTEGIT_SOURCES_DIR}/CDiffLine.h"
  "${CUTEGIT_SOURCES_DIR}/CDiffModel.h"
  "${CUTEGIT_SOURCES_DIR}/CDiffModelProxy.h"
  "${CUTEGIT_SOURCES_DIR}/CFlatFileModel.h"
  "${CUTEGIT_SOURCES_DIR}/CFlatFileModelProxy.h"
  "${CUTEGIT_SOURCES_DIR}/CStagedFileModelProxy.h"
  "${CUTEGIT_SOURCES_DIR}/CBranch.h"
  "${CUTEGIT_SOURCES_DIR}/CBranchModel.h"
  "${CUTEGIT_SOURCES_DIR}/CCommitLogModel.h"
  "${CUTEGIT_SOURCES_DIR}/CLogModel.h"
  "${CUTEGIT_SOURCES_DIR}/CLogModelProxy.h"
  "${CUTEGIT_SOURCES_DIR}/CLogLine.h"
  "${CUTEGIT_SOURCES_DIR}/CLogLineCollection.h"
  "${CUTEGIT_SOURCES_DIR}/CGraphModel.h"
  "${CUTEGIT_SOURCES_DIR}/CGraphLine.h"
  "${CUTEGIT_SOURCES_DIR}/CGraphLineCollection.h"
  "${CUTEGIT_SOURCES_DIR}/CRepoFile.h"
  "${CUTEGIT_SOURCES_DIR}/CLabel.h"
  "${CUTEGIT_SOURCES_DIR}/CLabelModel.h"
  "${CUTEGIT_SOURCES_DIR}/CEnums.h"
  "${CUTEGIT_SOURCES_DIR}/commands/CCommands.h"
  "${CUTEGIT_SOURCES_DIR}/commands/CGitCommands.h"
  "${CUTEGIT_SOURCES_DIR}/commands/CGerritCommands.h"
  "${CUTEGIT_SOURCES_DIR}/commands/CSvnCommands.h"
  "${CUTEGIT_SOURCES_DIR}/commands/CHgCommands.h"
  "${CUTEGIT_SOURCES_DIR}/commands/CExecution.h"
)

add_executable(tst_patchbuilder
  cpp/tst_patchbuilder.cpp
  ${PATCHBUILDER_HEADERS}
  ${PATCHBUILDER_SOURCES}
)
target_include_directories(tst_patchbuilder PRIVATE
  "${CUTEGIT_SOURCES_DIR}"
  "${QTPLUS_CPP_DIR}"
)
target_link_libraries(tst_patchbuilder PRIVATE Qt6::Core Qt6::Test Qt6::Xml)

add_executable(diff-qmlsetup
  qmlsetup/setup.cpp
  ${PATCHBUILDER_HEADERS}
  ${PATCHBUILDER_SOURCES}
)
target_include_directories(diff-qmlsetup PRIVATE
  "${CUTEGIT_SOURCES_DIR}"
  "${QTPLUS_CPP_DIR}"
)
target_link_libraries(diff-qmlsetup PRIVATE
  Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickTest Qt6::Test Qt6::Xml)

add_executable(tst_repository
  integration/tst_repository.cpp
  ${PATCHBUILDER_HEADERS}
  ${REPOSITORY_HEADERS}
  ${REPOSITORY_SOURCES}
)
target_include_directories(tst_repository PRIVATE
  "${CUTEGIT_SOURCES_DIR}"
  "${QTPLUS_CPP_DIR}"
)
target_link_libraries(tst_repository PRIVATE
  Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Test Qt6::Xml)

add_executable(tst_startup
  startup/tst_startup.cpp
  "${CUTEGIT_SOURCES_DIR}/CController.h"
  "${CUTEGIT_SOURCES_DIR}/CController.cpp"
  "${CUTEGIT_SOURCES_DIR}/CLayout.h"
  "${CUTEGIT_SOURCES_DIR}/CLayout.cpp"
  "${CUTEGIT_SOURCES_DIR}/CRepositoryModel.h"
  "${CUTEGIT_SOURCES_DIR}/CRepositoryModel.cpp"
  ${PATCHBUILDER_HEADERS}
  ${REPOSITORY_HEADERS}
  "${CUTEGIT_SOURCES_DIR}/CLayout.cpp"
  "${CUTEGIT_SOURCES_DIR}/CRepositoryModel.cpp"
  ${REPOSITORY_SOURCES}
  ../CuteGit/resources.qrc
)
target_include_directories(tst_startup PRIVATE
  "${CUTEGIT_SOURCES_DIR}"
  "${QTPLUS_CPP_DIR}"
)
target_compile_definitions(tst_startup PRIVATE
  "VERSION_STRING=\"${PROJECT_VERSION}\""
)
target_link_libraries(tst_startup PRIVATE
  Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Test Qt6::Xml
  Qt6::Qml Qt6::Quick Qt6::QuickControls2)

set_target_properties(tst_patchbuilder diff-qmlsetup tst_repository tst_startup
  PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)
