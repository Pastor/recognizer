#
# Find Tesseract
#
# Exported variables:
#    Tesseract_FOUND
#    Tesseract_INCLUDE_DIRS
#    Tesseract_LIBRARIES
#
#    Tesseract_VERSION
#    Tesseract_MAJOR_VERSION
#    Tesseract_MINOR_VERSION
#

find_path(Tesseract_INCLUDE_DIR tesseract/capi.h tesseract/ccutil/platform.h
    HINTS
    /usr/include
    /usr/local/include
    /opt/include
    /opt/local/include
    ${Tesseract_DIR}/include
)
if(NOT "${Tesseract_INCLUDE_DIR}" EQUAL "Tesseract_INCLUDE_DIR-NOTFOUND")
    set(Tesseract_INCLUDE_DIRS ${Tesseract_INCLUDE_DIR}/tesseract)
    file(STRINGS ${Tesseract_INCLUDE_DIRS}/capi.h Tesseract_MAJOR_VERSION REGEX "TESSERACT_MAJOR_VERSION")
    file(STRINGS ${Tesseract_INCLUDE_DIRS}/capi.h Tesseract_MINOR_VERSION REGEX "TESSERACT_MINOR_VERSION")
    string(REGEX MATCH "[0-9]+" Tesseract_MAJOR_VERSION ${Tesseract_MAJOR_VERSION})
    string(REGEX MATCH "[0-9]+" Tesseract_MINOR_VERSION ${Tesseract_MINOR_VERSION})
    set(Tesseract_VERSION ${Tesseract_MAJOR_VERSION}.${Tesseract_MINOR_VERSION})
endif()

find_library(Tesseract_LIBRARY NAMES tesseract libtesseract
    HINTS
    /usr/lib
    /usr/local/lib
    /opt/lib
    /opt/local/lib
    ${Tesseract_DIR}/lib
)
set(Tesseract_LIBRARIES ${Tesseract_LIBRARY})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Tesseract
    REQUIRED_VARS
        Tesseract_INCLUDE_DIRS
        Tesseract_LIBRARIES
    VERSION_VAR Tesseract_VERSION
    FAIL_MESSAGE "Try to set Tesseract_DIR or Tesseract_ROOT"
)

mark_as_advanced(Tesseract_INCLUDE_DIRS Tesseract_LIBRARIES)

