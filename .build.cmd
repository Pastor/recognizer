@echo off


rem x64
set platform="x86"
rem set platform="x64"
set buildtype=Release

set PATH=%PATH%;C:\cmake\bin
set cwd=%CD%
set downloader=%cwd%\.build\utils\bin\%buildtype%\hget.exe
set unzipper=%cwd%\utils\7za 
set zipper=%cwd%\utils\7za
set git=%githome%\git.exe

mkdir %cwd%\.build
mkdir %cwd%\.build\include
mkdir %cwd%\.build\lib

if %platform%=="x86" set vcplatform=Win32
if %platform%=="x86" set vcenvironment=x86
if %platform%=="x86" set generator="Visual Studio 14"
if %platform%=="x64" set vcplatform="x64"
if %platform%=="x64" set vcenvironment=amd64
if %platform%=="x64" set generator="Visual Studio 14 Win64"

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" %vcenvironment%


set INCLUDE=%cwd%\.build\include;%INCLUDE%
set LIB=%cwd%\.build\lib;%LIB%

echo   Platform: %platform%
echo VcPlatform: %vcplatform%
echo      Build: %buildtype%
echo  Generator: %generator%
echo        CWD: %cwd%
echo Downloader: %downloader%

call :DependLibraries
rem call :LeptonicaBuild
rem call :TesseractBuild
rem call :PngLibrary
rem call :TiffLibrary
rem call :GifLibrary
rem exit /b

rd /s /q %cwd%\.build\recognizer
mkdir %cwd%\.build\recognizer

cmake %cwd% -B%cwd%\.build\recognizer -G %generator% ^
                                      -DADD_INCLUDE_DIR=%cwd%\.build\tesseract-master\ccutil ^
                                      -DLeptonica_BUILD_DIR=%cwd%\.build\leptonica-master\build ^
                                      -DTesseract_BUILD_DIR=%cwd%\.build\tesseract-master\build ^
                                      -DMongoose_DIR=%cwd%\.build\mongoose-master ^
                                      -DCryptoAlgorithms_DIR=%cwd%\.build\crypto-algorithms-master ^
                                      -DJson_DIR=%cwd%\.build\json-master\src ^
                                      -DSQLite_DIR=%cwd%\.build\sqlite-master ^
                                      -DSTATIC=1 ^
                                      -DCMAKE_BUILD_TYPE=%buildtype%
msbuild %cwd%\.build\recognizer\recognizer.sln /p:Platform=%vcplatform% /p:ReleaseBuild=true /p:Configuration=%buildtype%

mkdir %cwd%\.build\recognizer\bin\%buildtype%\tessdata
xcopy /S /E /Y /C %cwd%\3rdparty\tesseract\tessdata %cwd%\.build\recognizer\bin\%buildtype%\tessdata
xcopy /S /E /Y /C %cwd%\.build\leptonica-master\build\bin\%buildtype%\*.dll %cwd%\.build\recognizer\bin\%buildtype%
xcopy /S /E /Y /C %cwd%\.build\tesseract-master\build\bin\%buildtype%\*.dll %cwd%\.build\recognizer\bin\%buildtype%

%zipper% a %cwd%\.build\restserver_%platform%.zip ^
           %cwd%\.build\recognizer\bin\%buildtype%\*.exe ^
           %cwd%\.build\recognizer\bin\%buildtype%\*.dll ^
           %cwd%\3rdparty\redist.vs2015\vc_redist.%platform%.exe ^
           %cwd%\.build\recognizer\bin\%buildtype%\tessdata ^
           %cwd%\.build\tesseract-master\build\bin\%buildtype%\tesseract.exe
%zipper% a %cwd%\.build\restserver-devel_%platform%.zip ^
           %cwd%\.build\include ^
           %cwd%\.build\lib

exit /b
rem ==========================================
:DependLibraries

echo Utils libtaties and programs
rd /s /q %CWD%\.build\utils
cmake -H%cwd%\utils -B%cwd%\.build\utils -G %generator% -DCMAKE_BUILD_TYPE=%buildtype%
msbuild %cwd%\.build\utils\utils.sln /p:Platform=%vcplatform% /p:ReleaseBuild=true /p:Configuration=%buildtype%

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
call :PngLibrary
call :TiffLibrary

cmake -H%cwd%\.build\leptonica-master -B%cwd%\.build\leptonica-master\build -G %generator% ^
      -DSTATIC=1 ^
      -DCMAKE_PREFIX_PATH=%cwd%\.build ^
      -DCMAKE_INCLUDE_PATH=%cwd%\.build\include ^
      -DJPEG_LIBRARY=%cwd%\.build\lib\jpeg.lib
msbuild %cwd%\.build\leptonica-master\build\leptonica.sln /p:Platform=%vcplatform% /p:ReleaseBuild=true /p:Configuration=%buildtype%
copy /V /Y %cwd%\.build\leptonica-master\build\src\%buildtype%\*.lib %cwd%\.build\lib

:TesseractBuild
cmake -H%cwd%\.build\tesseract-master -B%cwd%\.build\tesseract-master\build -DLeptonica_BUILD_DIR=%cwd%\.build\leptonica-master\build -G %generator% -DSTATIC=1
msbuild %cwd%\.build\tesseract-master\build\tesseract.sln /p:Platform=%vcplatform% /p:ReleaseBuild=true /p:Configuration=%buildtype%
copy /V /Y %cwd%\.build\tesseract-master\build\%buildtype%\*.lib %cwd%\.build\lib

exit /b

rem ============================ Jpeg library
:JpegLibrary
echo Jpeg library
del /q %cwd%\.build\libjpeg.zip
rd /s /q %cwd%\.build\jpeg-9b
%downloader% http://www.ijg.org/files/jpegsr9b.zip %cwd%\.build\libjpeg.zip
%unzipper% x %cwd%\.build\libjpeg.zip -o%cwd%\.build
copy /V /Y %cwd%\3rdparty\libjpeg\CMakeLists.txt %cwd%\.build\jpeg-9b\CMakeLists.txt
cmake -H%cwd%\.build\jpeg-9b -B%cwd%\.build\jpeg-9b\build -G %generator% -DCMAKE_BUILD_TYPE=%buildtype%
msbuild %cwd%\.build\jpeg-9b\build\jpeg.sln /p:Platform=%vcplatform% /p:ReleaseBuild=true /p:Configuration=%buildtype%
copy /V /Y %cwd%\.build\jpeg-9b\jconfig.h %cwd%\.build\include\jconfig.h
copy /V /Y %cwd%\.build\jpeg-9b\jerror.h %cwd%\.build\include\jerror.h
copy /V /Y %cwd%\.build\jpeg-9b\jpeglib.h %cwd%\.build\include\jpeglib.h
copy /V /Y %cwd%\.build\jpeg-9b\jmorecfg.h %cwd%\.build\include\jmorecfg.h
copy /V /Y %cwd%\.build\jpeg-9b\build\Release\jpeg.lib %cwd%\.build\lib\jpeg.lib

copy /V /Y %cwd%\.build\jpeg-9b\jconfig.h %cwd%\.build\leptonica-master\src\jconfig.h
copy /V /Y %cwd%\.build\jpeg-9b\jerror.h %cwd%\.build\leptonica-master\src\jerror.h
copy /V /Y %cwd%\.build\jpeg-9b\jpeglib.h %cwd%\.build\leptonica-master\src\jpeglib.h
copy /V /Y %cwd%\.build\jpeg-9b\jmorecfg.h %cwd%\.build\leptonica-master\src\jmorecfg.h
copy /V /Y %cwd%\.build\jpeg-9b\build\%buildtype%\jpeg.lib %cwd%\.build\lib\jpeg.lib

exit /b

:PngLibrary
echo Png library
del /q %cwd%\.build\libpng.zip
rd /s /q %cwd%\.build\libpng
rd /s /q %cwd%\.build\zlib
%downloader% ftp://ftp.simplesystems.org/pub/libpng/png/src/libpng16/lpng1621.zip %cwd%\.build\libpng.zip
%downloader% http://zlib.net/zlib128.zip %cwd%\.build\zlib.zip
%unzipper% x %cwd%\.build\libpng.zip -o%cwd%\.build
%unzipper% x %cwd%\.build\zlib.zip   -o%cwd%\.build
move /Y %cwd%\.build\lpng1621 %cwd%\.build\libpng
move /Y %cwd%\.build\zlib-1.2.8 %cwd%\.build\zlib
copy /V /Y %cwd%\3rdparty\libpng\CMakeLists.txt %cwd%\.build\libpng\CMakeLists.txt

cmake -H%cwd%\.build\zlib -B%cwd%\.build\zlib\build -G %generator% -DCMAKE_BUILD_TYPE=%buildtype%
msbuild %cwd%\.build\zlib\build\zlib.sln /p:Platform=%vcplatform% /p:ReleaseBuild=true /p:Configuration=%buildtype%
copy /V /Y %cwd%\.build\zlib\build\zconf.h %cwd%\.build\zlib\zconf.h
copy /V /Y %cwd%\.build\zlib\zlib.h %cwd%\.build\include\zlib.h
copy /V /Y %cwd%\.build\zlib\zconf.h %cwd%\.build\include\zconf.h
copy /V /Y %cwd%\.build\zlib\zlib.h %cwd%\.build\leptonica-master\src\zlib.h
copy /V /Y %cwd%\.build\zlib\zconf.h %cwd%\.build\leptonica-master\src\zconf.h
copy /V /Y %cwd%\.build\zlib\build\%buildtype%\zlibstatic.lib %cwd%\.build\lib\zlib.lib


cmake -H%cwd%\.build\libpng -B%cwd%\.build\libpng\build -G %generator% -DCMAKE_BUILD_TYPE=%buildtype% -DZLIB_LIBRARY=%cwd%\.build\zlib\build\%buildtype%\zlibstatic.lib -DZLIB_INCLUDE_DIR=%cwd%\.build\zlib
msbuild %cwd%\.build\libpng\build\png.sln /p:Platform=%vcplatform% /p:ReleaseBuild=true /p:Configuration=%buildtype%

copy /V /Y %cwd%\.build\libpng\png.h %cwd%\.build\include\png.h
copy /V /Y %cwd%\.build\libpng\pngconf.h %cwd%\.build\include\pngconf.h
copy /V /Y %cwd%\.build\libpng\pnglibconf.h %cwd%\.build\include\pnglibconf.h

copy /V /Y %cwd%\.build\libpng\png.h %cwd%\.build\leptonica-master\src\png.h
copy /V /Y %cwd%\.build\libpng\pngconf.h %cwd%\.build\leptonica-master\src\pngconf.h
copy /V /Y %cwd%\.build\libpng\pnglibconf.h %cwd%\.build\leptonica-master\src\pnglibconf.h
copy /V /Y %cwd%\.build\libpng\build\%buildtype%\png.lib %cwd%\.build\lib\png.lib

exit /b

:TiffLibrary
echo Tiff library
del    /q %cwd%\.build\libtiff.zip
rd  /s /q %cwd%\.build\libtiff
%downloader% http://dl.maptools.org/dl/libtiff/tiff-3.8.2.zip %cwd%\.build\libtiff.zip
%unzipper% x %cwd%\.build\libtiff.zip -o%cwd%\.build
move /Y %cwd%\.build\tiff-3.8.2 %cwd%\.build\libtiff
copy /V /Y %cwd%\3rdparty\libtiff\CMakeLists.txt %cwd%\.build\libtiff\CMakeLists.txt
cmake -H%cwd%\.build\libtiff -B%cwd%\.build\libtiff\build -G %generator% -DCMAKE_BUILD_TYPE=%buildtype% ^
                                                                         -DZLIB_LIBRARY=%cwd%\.build\lib\zlib.lib -DZLIB_INCLUDE_DIR=%cwd%\.build\include ^
                                                                         -DJPEG_LIBRARY=%cwd%\.build\lib\jpeg.lib -DJPEG_INCLUDE_DIR=%cwd%\.build\include
msbuild %cwd%\.build\libtiff\build\tiff.sln /p:Platform=%vcplatform% /p:ReleaseBuild=true /p:Configuration=%buildtype%

copy /V /Y %cwd%\.build\libtiff\libtiff\tiff.h     %cwd%\.build\include\tiff.h
copy /V /Y %cwd%\.build\libtiff\libtiff\tiffio.h   %cwd%\.build\include\tiffio.h
copy /V /Y %cwd%\.build\libtiff\libtiff\tiffvers.h %cwd%\.build\include\tiffvers.h
copy /V /Y %cwd%\.build\libtiff\tiffconf.h         %cwd%\.build\include\tiffconf.h
copy /V /Y %cwd%\.build\libtiff\tif_config.h       %cwd%\.build\include\tif_config.h
                                                   
copy /V /Y %cwd%\.build\include\tiff.h             %cwd%\.build\leptonica-master\src\tiff.h
copy /V /Y %cwd%\.build\include\tiffio.h           %cwd%\.build\leptonica-master\src\tiffio.h
copy /V /Y %cwd%\.build\include\tiffvers.h         %cwd%\.build\leptonica-master\src\tiffvers.h
copy /V /Y %cwd%\.build\include\tiffconf.h         %cwd%\.build\leptonica-master\src\tiffconf.h
copy /V /Y %cwd%\.build\include\tif_config.h       %cwd%\.build\leptonica-master\src\tif_config.h

copy /V /Y %cwd%\.build\libtiff\build\%buildtype%\tiff.lib %cwd%\.build\lib\tiff.lib

exit /b

:GifLibrary
rem http://sourceforge.net/projects/gnuwin32/files/giflib/4.1.4-1/giflib-4.1.4-1-src.zip/download
echo Gif library
del    /q %cwd%\.build\libgif.zip
rd  /s /q %cwd%\.build\libgif
%downloader% http://freefr.dl.sourceforge.net/project/gnuwin32/giflib/4.1.4-1/giflib-4.1.4-1-src.zip %cwd%\.build\libgif.zip
%unzipper% x %cwd%\.build\libgif.zip -o%cwd%\.build\libgif
move /Y %cwd%\.build\libgif\src\giflib\4.1.4\giflib-4.1.4-src\lib %cwd%\.build\libgif
copy /V /Y %cwd%\3rdparty\libgif\CMakeLists.txt %cwd%\.build\libgif\CMakeLists.txt
cmake -H%cwd%\.build\libgif -B%cwd%\.build\libgif\build -G %generator% -DCMAKE_BUILD_TYPE=%buildtype% 
msbuild %cwd%\.build\libgif\build\gif.sln /p:Platform=%vcplatform% /p:ReleaseBuild=true /p:Configuration=%buildtype%
copy /V /Y %cwd%\.build\libgif\lib\gif_lib.h %cwd%\.build\include\gif_lib.h
copy /V /Y %cwd%\.build\libgif\build\%buildtype%\gif.lib %cwd%\.build\lib\gif.lib
exit /b
