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

project(png C)

if (WIN32)
    string(REPLACE "\\" "/" ZLIB_LIBRARY      ${ZLIB_LIBRARY})
    #string(REPLACE "\\" "/" ZLIB_INCLIDE_DIR  ${ZLIB_INCLIDE_DIR})
endif()


set(LIBRARY_TYPE STATIC)
if (STATIC)
    set(LIBRARY_TYPE)
endif()

if (WIN32)
    if (MSVC)
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
        
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
add_definitions(-D_LIB)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/scripts/pnglibconf.h.prebuilt DESTINATION ${CMAKE_SOURCE_DIR})
file(RENAME ${CMAKE_SOURCE_DIR}/pnglibconf.h.prebuilt ${CMAKE_SOURCE_DIR}/pnglibconf.h)
set(png_src 
    pngerror.c
    pngget.c
    pngmem.c
    pngpread.c
    pngread.c
    pngrio.c
    pngrtran.c
    pngrutil.c
    pngset.c
    pngtrans.c
    pngwio.c
    pngwrite.c
    pngwtran.c
    pngwutil.c
    png.c
)

add_library  (png ${LIBRARY_TYPE} ${png_src} ${ZLIB_LIBRARY})
export(TARGETS png FILE ${CMAKE_BINARY_DIR}/PngTargets.cmake)

add_executable         (pngtest pngtest.c )
target_link_libraries  (pngtest png ${ZLIB_LIBRARY})
