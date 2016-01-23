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

project(gif C)

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
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR})
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/lib)

set(gif_src 
    ${CMAKE_SOURCE_DIR}/lib/dev2gif.c
    #${CMAKE_SOURCE_DIR}/lib/dgif_lib.c
    #${CMAKE_SOURCE_DIR}/lib/egif_lib.c
    #${CMAKE_SOURCE_DIR}/lib/getarg.c
    ${CMAKE_SOURCE_DIR}/lib/gif_err.c
    ${CMAKE_SOURCE_DIR}/lib/gif_font.c
    #${CMAKE_SOURCE_DIR}/lib/gif_hash.c
    ${CMAKE_SOURCE_DIR}/lib/gifalloc.c
    #${CMAKE_SOURCE_DIR}/lib/qprintf.c
    ${CMAKE_SOURCE_DIR}/lib/quantize.c
)

add_library  (gif ${LIBRARY_TYPE} ${gif_src})
export(TARGETS gif FILE ${CMAKE_BINARY_DIR}/GifTargets.cmake)

