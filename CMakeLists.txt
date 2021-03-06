cmake_minimum_required(VERSION 2.8.11)

# In-source builds are disabled.
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR
        "CMake generation is not possible within the source directory!"
        "\n Remove the CMakeCache.txt file and try again from another folder, e.g.:"
        "\n "
        "\n rm CMakeCache.txt"
        "\n mkdir build"
        "\n cd build"
        "\n cmake .."
    )
endif()

#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/cmake")

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}")

# Use solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake Targets")

###############################################################################
#
# project settings
#
###############################################################################

project(recognizer C CXX)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PLAIN ${VERSION_MAJOR}.${VERSION_MINOR})

set(MINIMUM_LEPTONICA_VERSION 1.71)
set(MINIMUM_TESSERACT_VERSION 3.02)
#set(SQLite_URL       "https://www.sqlite.org/snapshot/sqlite-amalgamation-201601141433.zip")
#set(SQLite_ARCHIVE   "${CMAKE_CURRENT_BINARY_DIR}/sqlite.zip")
#set(SQLite_HASH      "ce0f8700274488fbd4351dba2f82a2822125807d")
#set(SQLite_DIR       "${CMAKE_CURRENT_BINARY_DIR}/sqlite-current")

#message(WARNING "Leptonica: " ${Leptonica_BUILD_DIR})
if (NOT Leptonica_BUILD_DIR)
  find_package(Leptonica ${MINIMUM_LEPTONICA_VERSION} REQUIRED)
else()
  set(Leptonica_DIR ${Leptonica_BUILD_DIR})
  find_package(Leptonica ${MINIMUM_LEPTONICA_VERSION} REQUIRED CONFIG)
endif()

#message(WARNING "Tesseract: " ${Tesseract_BUILD_DIR})
if (NOT Tesseract_BUILD_DIR)
  find_package(Tesseract ${MINIMUM_LEPTONICA_VERSION} REQUIRED)
else()
  set(Tesseract_DIR ${Tesseract_BUILD_DIR})
  find_package(Tesseract ${MINIMUM_LEPTONICA_VERSION} REQUIRED CONFIG)
endif()

#find_package(OpenCL QUIET)
#find_package(PkgConfig)


#file(DOWNLOAD
#    ${SQLite_URL}
#    ${SQLite_ARCHIVE}
#    SHOW_PROGRESS
#    EXPECTED_HASH SHA1=${SQLite_HASH}
#)
#execute_process(COMMAND mkdir ${SQLite_DIR})
#execute_process(COMMAND ${CMAKE_COMMAND} -E tar xz "${SQLite_ARCHIVE}"
#    WORKING_DIRECTORY "${SQLite_DIR}"
#    RESULT_VARIABLE __result
#)
#if(NOT __result EQUAL 0)
#    message(FATAL_ERROR "error ${__result}")
#endif()


###############################################################################
#
# compiler and linker
#
###############################################################################

set(LIBRARY_TYPE SHARED)
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

###############################################################################
#
# configure
#
###############################################################################

include(Configure)

set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/api)

configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/templates/RecognizerConfig-version.cmake.in
    ${CMAKE_BINARY_DIR}/RecognizerConfig-version.cmake @ONLY)
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/templates/RecognizerConfig.cmake.in
    ${CMAKE_BINARY_DIR}/RecognizerConfig.cmake @ONLY) 

###############################################################################
#
# build
#
###############################################################################

include(BuildFunctions)
include(SourceGroups)

add_definitions(-DHAVE_CONFIG_H)
add_definitions(-D_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS=1)
add_definitions(-DUSE_STD_NAMESPACE=1)
add_definitions(-DMG_ENABLE_THREADS)
#add_definitions(-DWINDLLNAME="libtesseract${VERSION_MAJOR}${VERSION_MINOR}.dll")

include_directories(${Leptonica_INCLUDE_DIRS})
include_directories(${Tesseract_INCLUDE_DIRS})
include_directories(${ADD_INCLUDE_DIR})
include_directories(${CMAKE_BINARY_DIR})

if (WIN32)
    string(REPLACE "\\" "/" Json_DIR             ${Json_DIR})
    string(REPLACE "\\" "/" Mongoose_DIR         ${Mongoose_DIR})
    string(REPLACE "\\" "/" CryptoAlgorithms_DIR ${CryptoAlgorithms_DIR})
    string(REPLACE "\\" "/" SQLite_DIR           ${SQLite_DIR})
endif()

include_directories(restserver)
include_directories(${Json_DIR})
include_directories(${Mongoose_DIR})
include_directories(${CryptoAlgorithms_DIR})
include_directories(${SQLite_DIR})

########################################
# LIBRARY
########################################

#string(SUBSTRING ${VERSION_MINOR} 0 1 VERSION_MINOR_0)
#string(SUBSTRING ${VERSION_MINOR} 1 1 VERSION_MINOR_1)

file(GLOB restserver_src
    "restserver/*.cpp"
)
file(GLOB restserver_hdr
    "restserver/*.h"
)
#if (WIN32)
#    file(GLOB tesseract_win32_src "vs2010/port/*.cpp")
#    file(GLOB tesseract_win32_hdr "vs2010/port/*.h")
#    set(tesseract_src ${tesseract_src} ${tesseract_win32_src})
#    set(tesseract_hdr ${tesseract_hdr} ${tesseract_win32_hdr})
#endif()

set(restserver_src ${restserver_src}
     ${CryptoAlgorithms_DIR}/sha256.c
     ${Mongoose_DIR}/mongoose.c
     ${SQLite_DIR}/sqlite3.c
)
set(restserver_hdr ${restserver_hdr}
     ${CryptoAlgorithms_DIR}/sha256.h
     ${Mongoose_DIR}/mongoose.h
     ${Json_DIR}/json.hpp
)

#add_library                     (tesseract ${LIBRARY_TYPE} ${tesseract_src} ${tesseract_hdr})
#if (NOT STATIC)
#target_compile_definitions      (tesseract PUBLIC -DTESS_EXPORTS)
#endif()
#target_link_libraries           (tesseract ${Leptonica_LIBRARIES} ${LIB_Ws2_32} ${LIB_pthread})
#set_target_properties           (tesseract PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR_0}.${VERSION_MINOR_1})
#set_target_properties           (tesseract PROPERTIES SOVERSION ${VERSION_MAJOR}.${VERSION_MINOR_0}.${VERSION_MINOR_1})
#if (WIN32)
#set_target_properties           (tesseract PROPERTIES OUTPUT_NAME tesseract${VERSION_MAJOR}${VERSION_MINOR})
#set_target_properties           (tesseract PROPERTIES DEBUG_OUTPUT_NAME tesseract${VERSION_MAJOR}${VERSION_MINOR}d)
#endif()
#export(TARGETS tesseract FILE ${CMAKE_BINARY_DIR}/TesseractTargets.cmake)


########################################
# EXECUTABLE restserver
########################################

#set(tesseractmain_src
#    api/tesseractmain.cpp
#    vs2010/tesseract/resource.h
#    vs2010/tesseract/tesseract.rc
#)
add_executable                  (restserver ${restserver_src})
target_link_libraries           (restserver ${Leptonica_LIBRARIES} ${Tesseract_LIBRARIES} ${LIB_Ws2_32} ${LIB_pthread})
if(UNIX)
    target_link_libraries       (restserver dl)
endif()
set_target_properties           (restserver PROPERTIES OUTPUT_NAME restserver)
set_target_properties           (restserver PROPERTIES VERSION ${VERSION_MAJOR}.${VERSION_MINOR_0}.${VERSION_MINOR_1})
export(TARGETS restserver FILE ${CMAKE_BINARY_DIR}/RestServerTargets.cmake)

########################################

#add_subdirectory(training)

###############################################################################
