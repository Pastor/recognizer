@echo off

set PATH=%PATH%;C:\cmake\bin
set cwd=%CD%
set downloader=%cwd%\.build\utils\bin\Release\hget.exe
set unzipper=%cwd%\utils\7za 
set zipper=%cwd%\utils\7za
set git=%githome%\git.exe

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86

mkdir %cwd%\.build
mkdir %cwd%\.build\include
mkdir %cwd%\.build\lib

set INCLUDE=%cwd%\.build\include;%INCLUDE%
set LIB=%cwd%\.build\lib;%LIB%

call :DependLibraries
rem call :LeptonicaBuild


rd /s /q %cwd%\.build\recognizer
mkdir %cwd%\.build\recognizer

cmake %cwd% -B%cwd%\.build\recognizer -G "Visual Studio 14" -DADD_INCLUDE_DIR=%cwd%\.build\tesseract-master\ccutil ^
                                                            -DLeptonica_BUILD_DIR=%cwd%\.build\leptonica-master\build ^
                                                            -DTesseract_BUILD_DIR=%cwd%\.build\tesseract-master\build ^
                                                            -DMongoose_DIR=%cwd%\.build\mongoose-master ^
                                                            -DCryptoAlgorithms_DIR=%cwd%\.build\crypto-algorithms-master ^
                                                            -DJson_DIR=%cwd%\.build\json-master\src ^
                                                            -DSQLite_DIR=%cwd%\.build\sqlite-master ^
                                                            -DSTATIC=1
msbuild %cwd%\.build\recognizer\recognizer.sln /p:Platform=Win32 /p:ReleaseBuild=true /p:Configuration=Release

mkdir %cwd%\.build\recognizer\bin\Release\tessdata
xcopy /S /E /Y /C %cwd%\3rdparty\tesseract\tessdata %cwd%\.build\recognizer\bin\Release\tessdata
xcopy /S /E /Y /C %cwd%\.build\leptonica-master\build\bin\Release\*.dll %cwd%\.build\recognizer\bin\Release
xcopy /S /E /Y /C %cwd%\.build\tesseract-master\build\bin\Release\*.dll %cwd%\.build\recognizer\bin\Release

%zipper% a %cwd%\.build\restserver.zip ^
           %cwd%\.build\recognizer\bin\Release\*.exe ^
           %cwd%\.build\recognizer\bin\Release\*.dll ^
           %cwd%\3rdparty\redist.vs2015\vc_redist.x86.exe ^
           %cwd%\.build\recognizer\bin\Release\tessdata ^
           %cwd%\.build\tesseract-master\build\bin\Release\tesseract.exe

exit /b
rem ==========================================
:DependLibraries

echo Utils libtaties and programs
rd /s /q %CWD%\.build\utils
cmake -H%cwd%\utils -B%cwd%\.build\utils -G "Visual Studio 14" -DCMAKE_BUILD_TYPE=Release
msbuild %cwd%\.build\utils\utils.sln /p:Platform=Win32 /p:ReleaseBuild=true /p:Configuration=Release

del /q %cwd%\.build\leptonica-master.zip
del /q %cwd%\.build\tesseract-master.zip
del /q %cwd%\.build\mongoose-master.zip
del /q %cwd%\.build\json-master.zip
del /q %cwd%\.build\crypto-algorithms-master.zip
del /q %cwd%\.build\sqlite-master.zip

%downloader% https://github.com/egorpugin/leptonica/archive/master.zip            %cwd%\.build\leptonica-master.zip
%downloader% https://github.com/tesseract-ocr/tesseract/archive/master.zip        %cwd%\.build\tesseract-master.zip
%downloader% https://github.com/cesanta/mongoose/archive/master.zip               %cwd%\.build\mongoose-master.zip
%downloader% https://github.com/nlohmann/json/archive/master.zip                  %cwd%\.build\json-master.zip
%downloader% https://github.com/B-Con/crypto-algorithms/archive/master.zip        %cwd%\.build\crypto-algorithms-master.zip
%downloader% https://www.sqlite.org/snapshot/sqlite-amalgamation-201601141433.zip %cwd%\.build\sqlite-master.zip

rd /s /q %cwd%\.build\leptonica-master
rd /s /q %cwd%\.build\tesseract-master
rd /s /q %cwd%\.build\mongoose-master
rd /s /q %cwd%\.build\json-master
rd /s /q %cwd%\.build\crypto-algorithms-master
rd /s /q %cwd%\.build\sqlite-master

%unzipper% x %cwd%\.build\leptonica-master.zip          -o%cwd%\.build
%unzipper% x %cwd%\.build\tesseract-master.zip          -o%cwd%\.build
%unzipper% x %cwd%\.build\mongoose-master.zip           -o%cwd%\.build
%unzipper% x %cwd%\.build\json-master.zip               -o%cwd%\.build
%unzipper% x %cwd%\.build\crypto-algorithms-master.zip  -o%cwd%\.build
%unzipper% x %cwd%\.build\sqlite-master.zip             -o%cwd%\.build\sqlite-master

:LeptonicaBuild
call :JpegLibrary
cmake -H%cwd%\.build\leptonica-master -B%cwd%\.build\leptonica-master\build -G "Visual Studio 14" ^
      -DSTATIC=1 ^
      -DCMAKE_PREFIX_PATH=%cwd%\.build ^
      -DCMAKE_INCLUDE_PATH=%cwd%\.build\include ^
      -DJPEG_LIBRARY=%cwd%\.build\lib\jpeg.lib
msbuild %cwd%\.build\leptonica-master\build\leptonica.sln /p:Platform=Win32 /p:ReleaseBuild=true /p:Configuration=Release
copy /V /Y %cwd%\.build\leptonica-master\build\src\Release\*.lib %cwd%\.build\lib

cmake -H%cwd%\.build\tesseract-master -B%cwd%\.build\tesseract-master\build -DLeptonica_BUILD_DIR=%cwd%\.build\leptonica-master\build -G "Visual Studio 14" -DSTATIC=1
msbuild %cwd%\.build\tesseract-master\build\tesseract.sln /p:Platform=Win32 /p:ReleaseBuild=true /p:Configuration=Release
copy /V /Y %cwd%\.build\tesseract-master\build\src\Release\*.lib %cwd%\.build\lib

exit /b

rem ============================ Jpeg library
:JpegLibrary
echo Jpeg library
del /q %cwd%\.build\libjpeg.zip
rd /s /q %cwd%\.build\jpeg-9b
%downloader% http://www.ijg.org/files/jpegsr9b.zip %cwd%\.build\libjpeg.zip
%unzipper% x %cwd%\.build\libjpeg.zip -o%cwd%\.build
copy /V /Y %cwd%\3rdparty\libjpeg\CMakeLists.txt %cwd%\.build\jpeg-9b\CMakeLists.txt
cmake -H%cwd%\.build\jpeg-9b -B%cwd%\.build\jpeg-9b\build -G "Visual Studio 14" -DCMAKE_BUILD_TYPE=Release
msbuild %cwd%\.build\jpeg-9b\build\jpeg.sln /p:Platform=Win32 /p:ReleaseBuild=true /p:Configuration=Release
copy /V /Y %cwd%\.build\jpeg-9b\jconfig.h %cwd%\.build\include\jconfig.h
copy /V /Y %cwd%\.build\jpeg-9b\jerror.h %cwd%\.build\include\jerror.h
copy /V /Y %cwd%\.build\jpeg-9b\jpeglib.h %cwd%\.build\include\jpeglib.h
copy /V /Y %cwd%\.build\jpeg-9b\jmorecfg.h %cwd%\.build\include\jmorecfg.h
copy /V /Y %cwd%\.build\jpeg-9b\build\Release\jpeg.lib %cwd%\.build\lib\jpeg.lib

copy /V /Y %cwd%\.build\jpeg-9b\jconfig.h %cwd%\.build\leptonica-master\src\jconfig.h
copy /V /Y %cwd%\.build\jpeg-9b\jerror.h %cwd%\.build\leptonica-master\src\jerror.h
copy /V /Y %cwd%\.build\jpeg-9b\jpeglib.h %cwd%\.build\leptonica-master\src\jpeglib.h
copy /V /Y %cwd%\.build\jpeg-9b\jmorecfg.h %cwd%\.build\leptonica-master\src\jmorecfg.h
copy /V /Y %cwd%\.build\jpeg-9b\build\Release\jpeg.lib %cwd%\.build\lib\jpeg.lib

exit /b


