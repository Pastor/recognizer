@echo off

set PATH=%PATH%;C:\cmake\bin

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86

rd /s /q leptonica-master
rd /s /q tesseract-master

7za x leptonica-master.zip
7za x tesseract-master.zip
cmake -Hleptonica-master -Bleptonica-master/build -G "Visual Studio 14" -DSTATIC=1
msbuild leptonica-master/build/leptonica.sln /p:Platform=Win32 
cmake -Htesseract-master -Btesseract-master/build -G "Visual Studio 14" -DLeptonica_BUILD_DIR=%CWD%/leptonica-master/build -DSTATIC=1
msbuild tesseract-master/build/tesseract.sln /p:Platform=Win32 

rem /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll"