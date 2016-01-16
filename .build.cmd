@echo off

set PATH=%PATH%;C:\cmake\bin
set cwd=%CD%
set downloader=%cwd%\.build\utils\bin\Release\hget.exe
set unzipper=%cwd%\utils\7za 
set zipper=%cwd%\utils\7za

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86

mkdir %cwd%\.build
rem call :DependLibraries

rem rd /s /q %cwd%\.build\recognizer
rem mkdir %cwd%\.build\recognizer

rem cmake %cwd% -B%cwd%\.build\recognizer -G "Visual Studio 14" -DADD_INCLUDE_DIR=%cwd%\.build\tesseract-master\ccutil -DLeptonica_BUILD_DIR=%cwd%\.build\leptonica-master\build -DTesseract_BUILD_DIR=%cwd%\.build\tesseract-master\build -DSTATIC=1
rem msbuild %cwd%\.build\recognizer\recognizer.sln /p:Platform=Win32 /p:ReleaseBuild=true /p:Configuration=Release

mkdir %cwd%\.build\recognizer\bin\Release\tessdata
xcopy /S /E /Y /C %cwd%\3rdparty\tesseract\tessdata %cwd%\.build\recognizer\bin\Release\tessdata

%zipper% a %cwd%\.build\restserver.zip %cwd%\.build\recognizer\bin\Release\restserver.exe %cwd%\3rdparty\redist.vs2015\vc_redist.x86.exe %cwd%\.build\recognizer\bin\Release\tessdata


exit /b
rem ==========================================
:DependLibraries

rd /s /q %CWD%\.build\utils
cmake -H%cwd%\utils -B%cwd%\.build\utils -G "Visual Studio 14" -DSTATIC=1 -DCMAKE_BUILD_TYPE=Release
msbuild %cwd%\.build\utils\utils.sln /p:Platform=Win32 /p:ReleaseBuild=true /p:Configuration=Release


del /q %cwd%\.build\leptonica-master.zip
del /q %cwd%\.build\tesseract-master.zip

%downloader% https://github.com/egorpugin/leptonica/archive/master.zip      %cwd%\.build\leptonica-master.zip
%downloader% https://github.com/tesseract-ocr/tesseract/archive/master.zip  %cwd%\.build\tesseract-master.zip

rd /s /q %cwd%\.build\leptonica-master
rd /s /q %cwd%\.build\tesseract-master


%unzipper% x %cwd%\.build\leptonica-master.zip -o%cwd%\.build
%unzipper% x %cwd%\.build\tesseract-master.zip -o%cwd%\.build

cmake -H%cwd%\.build\leptonica-master -B%cwd%\.build\leptonica-master\build -G "Visual Studio 14" -DSTATIC=1
msbuild %cwd%\.build\leptonica-master\build\leptonica.sln /p:Platform=Win32 /p:ReleaseBuild=true /p:Configuration=Release

cmake -H%cwd%\.build\tesseract-master -B%cwd%\.build\tesseract-master\build -DLeptonica_BUILD_DIR=%cwd%\.build\leptonica-master\build -G "Visual Studio 14" -DSTATIC=1
msbuild %cwd%\.build\tesseract-master\build\tesseract.sln /p:Platform=Win32 /p:ReleaseBuild=true /p:Configuration=Release

exit /b