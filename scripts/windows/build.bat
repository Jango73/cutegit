@echo off
setlocal EnableExtensions EnableDelayedExpansion

set "EXIT_SUCCESS=0"
set "EXIT_FAILURE=1"
set "VERBOSE_ENABLED=1"
set "VERBOSE_DISABLED=0"

set "SCRIPT_FOLDER=%~dp0"
for %%I in ("%SCRIPT_FOLDER%..\..") do set "PROJECT_ROOT_FOLDER=%%~fI\"
set "BUILD_TYPE=Release"
set "VERBOSE_BUILD=%VERBOSE_DISABLED%"

:parse
if "%~1"=="" goto done

if /I "%~1"=="--debug" (
  set "BUILD_TYPE=Debug"
  shift
  goto parse
)
if /I "%~1"=="-d" (
  set "BUILD_TYPE=Debug"
  shift
  goto parse
)
if /I "%~1"=="--release" (
  set "BUILD_TYPE=Release"
  shift
  goto parse
)
if /I "%~1"=="-r" (
  set "BUILD_TYPE=Release"
  shift
  goto parse
)
if /I "%~1"=="--verbose" (
  set "VERBOSE_BUILD=%VERBOSE_ENABLED%"
  shift
  goto parse
)
if /I "%~1"=="-v" (
  set "VERBOSE_BUILD=%VERBOSE_ENABLED%"
  shift
  goto parse
)
if /I "%~1"=="-c" (
  if "%~2"=="" (
    echo Option -c requires an argument. 1>&2
    call :usage 1>&2
    exit /b %EXIT_FAILURE%
  )
  set "BUILD_TYPE=%~2"
  shift
  shift
  goto parse
)
if /I "%~1"=="-h" (
  call :usage
  exit /b %EXIT_SUCCESS%
)
if /I "%~1"=="--help" (
  call :usage
  exit /b %EXIT_SUCCESS%
)

echo Unknown option: %~1 1>&2
call :usage 1>&2
exit /b %EXIT_FAILURE%

:done
if /I not "%BUILD_TYPE%"=="Debug" if /I not "%BUILD_TYPE%"=="Release" (
  echo Invalid build type: %BUILD_TYPE% (use Debug or Release) 1>&2
  exit /b %EXIT_FAILURE%
)

if /I "%BUILD_TYPE%"=="Debug" (
  set "BUILD_FOLDER=%PROJECT_ROOT_FOLDER%build-debug"
  set "CONFIG_ARG=CONFIG+=debug"
) else (
  set "BUILD_FOLDER=%PROJECT_ROOT_FOLDER%build-release"
  set "CONFIG_ARG=CONFIG+=release"
)

rem Build qt-plus if it exists
set "QT_PLUS_DIR=%PROJECT_ROOT_FOLDER%qt-plus"
set "QT_PLUS_BUILD=%QT_PLUS_DIR%\scripts\windows\build.bat"
if exist "%QT_PLUS_DIR%" if exist "%QT_PLUS_BUILD%" (
  echo Building qt-plus library...
  if %VERBOSE_BUILD%==%VERBOSE_ENABLED% (
    call "%QT_PLUS_BUILD%" -c "%BUILD_TYPE%" -v
  ) else (
    call "%QT_PLUS_BUILD%" -c "%BUILD_TYPE%"
  )
  echo qt-plus built successfully.
) else (
  echo Warning: qt-plus not found or missing scripts\windows\build.bat 1>&2
)

if not exist "%BUILD_FOLDER%" mkdir "%BUILD_FOLDER%"
cd /d "%BUILD_FOLDER%"

rem Build stamp: regenerated on every build so a running binary can always be identified
for /f "usebackq delims=" %%T in (`powershell -NoProfile -Command "Get-Date -Format o"`) do set "BUILD_STAMP=%%T"
if not defined BUILD_STAMP set "BUILD_STAMP=unknown"
(
  echo #pragma once
  echo #define BUILD_TIMESTAMP "%BUILD_STAMP%"
) > "%BUILD_FOLDER%\CuteGit\BuildInfo.h"
copy /b "%PROJECT_ROOT_FOLDER%CuteGit\sources\main.cpp" +,, >nul
echo Build stamp: %BUILD_STAMP%

echo Running qmake with %CONFIG_ARG%...
qmake "%PROJECT_ROOT_FOLDER%\CuteGit.pro" %CONFIG_ARG%

if %VERBOSE_BUILD%==%VERBOSE_ENABLED% (
  make VERBOSE=1
) else (
  make
)

rem Descriptive UI layouts live next to the binary
if exist "%PROJECT_ROOT_FOLDER%CuteGit\deploy\layouts.json" (
  if not exist "%BUILD_FOLDER%\CuteGit\bin" mkdir "%BUILD_FOLDER%\CuteGit\bin"
  copy /y "%PROJECT_ROOT_FOLDER%CuteGit\deploy\layouts.json" "%BUILD_FOLDER%\CuteGit\bin\layouts.json" >nul
  echo Copied layouts.json next to the binary.
)

echo.
echo Built target in %BUILD_FOLDER%
exit /b %EXIT_SUCCESS%

:usage
echo Usage: build.bat [--debug^|--release] [-d^|-r] [--verbose^|-v] [-c Debug^|Release] [-h^|--help]
echo   --debug,   -d  Build type Debug
echo   --release, -r  Build type Release (default)
echo   --verbose, -v  Verbose build output
echo   -c             Build type (Debug or Release)
exit /b %EXIT_SUCCESS%
