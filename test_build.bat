@REM OcarSlicer build script for Windows
@echo off
set WP=%CD%

@REM Pack deps
if "%1"=="pack" (
    setlocal ENABLEDELAYEDEXPANSION 
    cd %WP%/deps/build
    for /f "tokens=2-4 delims=/ " %%a in ('date /t') do set build_date=%%c%%b%%a
    echo packing deps: CrealityPrint_dep_win64_!build_date!_vs2022.zip

    %WP%/tools/7z.exe a CrealityPrint_dep_win64_!build_date!_vs2022.zip CrealityPrint_dep
    exit /b 0
)

set debug=OFF
set debuginfo=OFF
if "%1"=="debug" set debug=ON
if "%2"=="debug" set debug=ON
if "%1"=="debuginfo" set debuginfo=ON
if "%2"=="debuginfo" set debuginfo=ON
if "%debug%"=="ON" (
    set build_type=Debug
    set build_dir=build-dbg
) else (
    if "%debuginfo%"=="ON" (
        set build_type=RelWithDebInfo
        set build_dir=build-dbginfo
    ) else (
        set build_type=Release
        set build_dir=build
    )
)
echo build type set to %build_type%

setlocal DISABLEDELAYEDEXPANSION 
cd deps
mkdir %build_dir%
cd %build_dir%
set DEPS=%CD%/OrcaSlicer_dep

if "%1"=="slicer" (
    GOTO :slicer
)
echo "building deps.."

echo cmake ../ -G "Visual Studio 17 2022" -A x64 -DDESTDIR="%CD%/OrcaSlicer_dep" -DCMAKE_BUILD_TYPE=%build_type% -DDEP_DEBUG=%debug% -DORCA_INCLUDE_DEBUG_INFO=%debuginfo% -DENABLE_BREAKPAD=OFF
cmake ../ -G "Visual Studio 17 2022" -A x64 -DDESTDIR="%CD%/OrcaSlicer_dep" -DCMAKE_BUILD_TYPE=%build_type% -DDEP_DEBUG=%debug% -DORCA_INCLUDE_DEBUG_INFO=%debuginfo% -DENABLE_BREAKPAD=OFF
cmake --build . --config %build_type% --target deps -- -m

if "%1"=="deps" exit /b 0

:slicer
echo "building Orca Slicer..."
cd %WP%
mkdir %build_dir%
cd %build_dir%

echo cmake .. -G "Visual Studio 17 2022" -A x64 -DBBL_RELEASE_TO_PUBLIC=1 -DCMAKE_PREFIX_PATH="%DEPS%/usr/local" -DCMAKE_INSTALL_PREFIX="./OrcaSlicer" -DCMAKE_BUILD_TYPE=%build_type% -DENABLE_BREAKPAD=OFF
cmake .. -G "Visual Studio 17 2022" -A x64 -DBBL_RELEASE_TO_PUBLIC=1 -DCMAKE_PREFIX_PATH="%DEPS%/usr/local" -DCMAKE_INSTALL_PREFIX="./OrcaSlicer" -DCMAKE_BUILD_TYPE=%build_type% -DWIN10SDK_PATH="C:/Program Files (x86)/Windows Kits/10/Include/10.0.22000.0" -DENABLE_BREAKPAD=OFF
cmake --build . --config %build_type% --target ALL_BUILD -- -m
cd ..
call run_gettext.bat
cd %build_dir%
cmake --build . --target install --config %build_type%
