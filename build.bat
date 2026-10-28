@echo off
setlocal

rem JUCE and the build tree live in a short path: Windows' 260-character path
rem limit breaks the build in deeply nested folders.
set "WORK=%LOCALAPPDATA%\Temp\gm"
set "SRC=%~dp0."
set "BUILD=%WORK%\build"
if not defined JUCE_DIR set "JUCE_DIR=%WORK%\JUCE"

if not exist "%JUCE_DIR%\CMakeLists.txt" (
    git clone -c core.longpaths=true --depth 1 --branch 8.0.4 https://github.com/juce-framework/JUCE.git "%JUCE_DIR%" || exit /b 1
)

cd /d "%WORK%"
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul || exit /b 1

cmake -S "%SRC%" -B "%BUILD%" -G Ninja -DCMAKE_BUILD_TYPE=Release -DJUCE_DIR="%JUCE_DIR%" || exit /b 1
cmake --build "%BUILD%" || exit /b 1

echo.
echo Built: %BUILD%\GlassMute_artefacts\Release\VST3\Big Ugly Pink Mute Plugin.vst3
