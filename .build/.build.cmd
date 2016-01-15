@echo off

set PATH=%PATH%;C:\cmake\bin

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86

rd /s /q leptonica-master
rd /s /q tesseract-master
rd /s /q build

7za x leptonica-master.zip
7za x tesseract-master.zip
set CWD=%CD%
cmake -Hleptonica-master -Bleptonica-master/build -G "Visual Studio 14" -DSTATIC=1
msbuild leptonica-master/build/leptonica.sln /p:Platform=Win32
cmake -Htesseract-master -Btesseract-master/build -G "Visual Studio 14" -DLeptonica_BUILD_DIR=%CWD%/leptonica-master/build -DSTATIC=1
msbuild tesseract-master/build/tesseract.sln /p:Platform=Win32 
mkdir build
cmake .. -B%CWD%/build -G "Visual Studio 14" -DADD_INCLUDE_DIR=%CWD%/tesseract-master/ccutil -DLeptonica_BUILD_DIR=%CWD%/leptonica-master/build -DTesseract_BUILD_DIR=%CWD%/tesseract-master/build -DSTATIC=1
msbuild %CWD%/build/recognizer.sln /p:Platform=Win32 
rem /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll"