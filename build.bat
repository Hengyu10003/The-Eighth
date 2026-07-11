@echo off
echo Building The Eighth...

set QT_PATH=C:\Qt\Qt5.9.9\5.9.9\mingw53_32

if not exist "%QT_PATH%\bin\qmake.exe" (
    echo Qt5.9.9 not found at %QT_PATH%
    echo Please set QT_PATH to your Qt5.9.9 installation path
    pause
    exit /b 1
)

set PATH=%QT_PATH%\bin;%PATH%

if not exist release (
    mkdir release
)

cd /d "%~dp0"

echo Running qmake...
qmake The_eighth.pro -spec win32-g++ CONFIG+=release

echo Running mingw32-make...
mingw32-make -j4

echo Build completed!
pause