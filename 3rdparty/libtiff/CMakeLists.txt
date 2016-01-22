cmake_minimum_required(VERSION 2.8.11)

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake Targets")

###############################################################################
#
# project settings
#
###############################################################################

project(tiff C)

if (WIN32)
    string(REPLACE "\\" "/" ZLIB_LIBRARY      ${ZLIB_LIBRARY})
    string(REPLACE "\\" "/" JPEG_LIBRARY      ${JPEG_LIBRARY})
endif()


set(LIBRARY_TYPE STATIC)
if (STATIC)
    set(LIBRARY_TYPE)
endif()

if (WIN32)
    if (MSVC)
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
        add_definitions(/wd4244 /wd4267 /wd4018 /wd4355 /wd4800 /wd4251 /wd4996 /wd4146 /wd4305)
        
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    endif()
    
    set(LIB_Ws2_32 Ws2_32)
endif()

if (CYGWIN)
    add_definitions(-D__CYGWIN__)
endif()

if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")
    
    set(LIB_pthread pthread)
endif()
include_directories(${ZLIB_INCLUDE_DIR})
include_directories(${JPEG_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/libtiff)
include_directories(${CMAKE_SOURCE_DIR})
add_definitions(-D_LIB)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR})

file(COPY ${CMAKE_SOURCE_DIR}/libtiff/tif_config.h.vc DESTINATION ${CMAKE_SOURCE_DIR})
file(RENAME ${CMAKE_SOURCE_DIR}/tif_config.h.vc ${CMAKE_SOURCE_DIR}/tif_config.h)

file(COPY ${CMAKE_SOURCE_DIR}/libtiff/tiffconf.h.vc DESTINATION ${CMAKE_SOURCE_DIR})
file(RENAME ${CMAKE_SOURCE_DIR}/tiffconf.h.vc ${CMAKE_SOURCE_DIR}/tiffconf.h)

set(tiff_src 
    ${CMAKE_SOURCE_DIR}/libtiff/tif_aux.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_close.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_codec.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_color.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_compress.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_dir.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_dirinfo.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_dirread.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_dirwrite.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_dumpmode.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_error.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_extension.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_fax3.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_fax3sm.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_getimage.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_jpeg.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_ojpeg.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_flush.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_luv.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_lzw.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_next.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_open.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_packbits.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_pixarlog.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_predict.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_print.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_read.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_stream.cxx
    ${CMAKE_SOURCE_DIR}/libtiff/tif_swab.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_strip.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_thunder.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_tile.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_version.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_warning.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_write.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_zip.c
    ${CMAKE_SOURCE_DIR}/libtiff/tif_unix.c
)

add_library  (tiff ${LIBRARY_TYPE} ${tiff_src} ${ZLIB_LIBRARY} ${JPEG_LIBRARY})
export(TARGETS tiff FILE ${CMAKE_BINARY_DIR}/TiffTargets.cmake)

