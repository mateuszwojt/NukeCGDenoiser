# - CMake find module for Nuke
#
# If requesting a specific release, the Nuke version string must be converted
# to a CMake-compatible version number before being passed to `find_package`.
# This should be done as follows:
#  6.3v8      ->   6.3.8
#  7.0v1b100  ->   7.0.1.100
#
# Input variables:
#  Nuke_ROOT
#
# Output variables:
#  NUKE_FOUND
#  NUKE_EXECUTABLE
#  NUKE_INCLUDE_DIRS
#  NUKE_LIBRARY_DIRS
#  NUKE_LIBRARIES
#  NUKE_DDIMAGE_LIBRARY
#  NUKE_VERSION_MAJOR
#  NUKE_VERSION_MINOR
#  NUKE_VERSION_RELEASE
#

if (TARGET Nuke::NDK)
    return()
endif()

set(_nuke_KNOWN_VERSIONS 9.0 10.0 10.5 11.0 11.1 11.2 11.3 12.0 12.1 12.2 13.0 13.1 14.0 14.1 15.0)
set(_nuke_TEST_VERSIONS) # List of Nuke-style strings (e.g. "7.0v4")


# If Nuke_ROOT is set, don't even bother with anything else
if(Nuke_ROOT)
    set(_nuke_TEST_PATHS ${Nuke_ROOT})
else()
    # TODO: Macro for duplicated nested loop code? (to generate permutations)
    if(Nuke_FIND_VERSION)
        if(Nuke_FIND_VERSION_EXACT)
            if(Nuke_FIND_VERSION_COUNT LESS 3)
                # An "exact" version was requested, but we weren't given a release.
                message(SEND_ERROR "'Exact' Nuke version requested, but no release specified. Nuke will not be found.")
            endif()
            set(_nuke_VERSION_STRING "${Nuke_FIND_VERSION_MAJOR}.${Nuke_FIND_VERSION_MINOR}v${Nuke_FIND_VERSION_PATCH}")
            if(Nuke_FIND_VERSION_TWEAK)
                # Beta version
                set(_nuke_VERSION_STRING "${_nuke_VERSION_STRING}b${Nuke_FIND_VERSION_TWEAK}")
            endif()
            list(APPEND _nuke_TEST_VERSIONS ${_nuke_VERSION_STRING})
        else()
            if(Nuke_FIND_VERSION_COUNT LESS 3)
                # Partial version
                if(Nuke_FIND_VERSION_COUNT EQUAL 1)
                    # E.g. 6
                    set(_nuke_FIND_MAJORMINOR "${Nuke_FIND_VERSION}.0")
                    set(_nuke_VERSION_PATTERN "^${Nuke_FIND_VERSION}\\.[0-9]$")
                    # Go for highest 6.x version
                    list(REVERSE _nuke_KNOWN_VERSIONS)
                elseif(Nuke_FIND_VERSION_COUNT EQUAL 2)
                    # E.g. 6.3
                    set(_nuke_FIND_MAJORMINOR ${Nuke_FIND_VERSION})
                    set(_nuke_VERSION_PATTERN "^${Nuke_FIND_VERSION_MAJOR}\\.${Nuke_FIND_VERSION_MINOR}$")
                endif()

                foreach(_known_version ${_nuke_KNOWN_VERSIONS})
                    # To avoid the need to keep this module up to date with the full Nuke
                    # release list, we just build a list of possible releases for the
                    # MAJOR.MINOR pair (currently using possible release versions v1-v13)
                    # We don't try and auto-locate beta versions.
                    string(REGEX MATCH ${_nuke_VERSION_PATTERN} _nuke_VERSION_PREFIX ${_known_version})
                    if(_nuke_VERSION_PREFIX)
                        if(NOT ${_known_version} VERSION_LESS ${_nuke_FIND_MAJORMINOR})
                            foreach(_release_num RANGE 13 1 -1)
                                list(APPEND _nuke_TEST_VERSIONS "${_known_version}v${_release_num}")
                            endforeach()
                        endif()
                    endif()
                endforeach()
            else()
                # Full version or beta
                set(_nuke_VERSION_STRING "${Nuke_FIND_VERSION_MAJOR}.${Nuke_FIND_VERSION_MINOR}v${Nuke_FIND_VERSION_PATCH}")
                if(Nuke_FIND_VERSION_TWEAK)
                    # Beta version
                    set(_nuke_VERSION_STRING "${_nuke_VERSION_STRING}b${Nuke_FIND_VERSION_TWEAK}")
                endif()
                list(APPEND _nuke_TEST_VERSIONS ${_nuke_VERSION_STRING})
            endif()
        endif()
    else()
        # If we're just grabbing any available version, we want the *highest* one
        # we can find, so flip the known versions list.
        list(REVERSE _nuke_KNOWN_VERSIONS)
        foreach(_known_version ${_nuke_KNOWN_VERSIONS})
            foreach(_release_num RANGE 13 1 -1)
                list(APPEND _nuke_TEST_VERSIONS "${_known_version}v${_release_num}")
            endforeach()
        endforeach()
    endif()

    if(APPLE)
        set(_nuke_TEMPLATE_PATH "/Applications/Nuke<VERSION>/Nuke<VERSION>.app/Contents/MacOS")
    elseif(WIN32)
        set(_nuke_TEMPLATE_PATH "C:/Program Files/Nuke<VERSION>")
    else() # Linux
        set(_nuke_TEMPLATE_PATH "/usr/local/Nuke<VERSION>")
    endif()

    foreach(_test_version ${_nuke_TEST_VERSIONS})
        string(REPLACE "<VERSION>" ${_test_version} _test_path ${_nuke_TEMPLATE_PATH})
        list(APPEND _nuke_TEST_PATHS ${_test_path})
    endforeach()
endif()


# Base search around DDImage, since its name is unversioned
find_library(NUKE_DDIMAGE_LIBRARY DDImage
    PATHS ${_nuke_TEST_PATHS}
    DOC "Nuke DDImage library path"
    NO_SYSTEM_ENVIRONMENT_PATH)

# Sanity-check to avoid a bunch of redundant errors.
if(NUKE_DDIMAGE_LIBRARY)
    get_filename_component(NUKE_LIBRARY_DIRS ${NUKE_DDIMAGE_LIBRARY} PATH)

    find_path(NUKE_INCLUDE_DIRS DDImage/Op.h "${NUKE_LIBRARY_DIRS}/include")

    # Pull version information from header
    # (We could pull the DDImage path apart instead, but this avoids dealing
    # with platform-specific naming.)
    file(STRINGS "${NUKE_INCLUDE_DIRS}/DDImage/ddImageVersionNumbers.h" _nuke_DDIMAGE_VERSION_H)
    string(REGEX REPLACE ".*#define kDDImageVersionMajorNum ([0-9]+).*" "\\1"
        NUKE_VERSION_MAJOR ${_nuke_DDIMAGE_VERSION_H})
    string(REGEX REPLACE ".*#define kDDImageVersionMinorNum ([0-9]+).*" "\\1"
        NUKE_VERSION_MINOR ${_nuke_DDIMAGE_VERSION_H})
    string(REGEX REPLACE ".*#define kDDImageVersionReleaseNum ([0-9]+).*" "\\1"
        NUKE_VERSION_RELEASE ${_nuke_DDIMAGE_VERSION_H})

    find_program(NUKE_EXECUTABLE
        NAMES
            Nuke
            "Nuke${NUKE_VERSION_MAJOR}.${NUKE_VERSION_MINOR}"
            "Nuke${NUKE_VERSION_MAJOR}.${NUKE_VERSION_MINOR}v${NUKE_VERSION_RELEASE}"
        PATHS ${NUKE_LIBRARY_DIRS}
        NO_SYSTEM_ENVIRONMENT_PATH
        DOC "Nuke executable path")
endif()

# Finalize search
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Nuke DEFAULT_MSG
    NUKE_DDIMAGE_LIBRARY NUKE_INCLUDE_DIRS NUKE_LIBRARY_DIRS NUKE_EXECUTABLE)


# The above code makes old-style cmake targets, but Nuke now has its own CMake config and we'd like to be compatible.
# Make targets which correspond to those in Nuke's own NukeConfig.cmake.
get_filename_component(NUKE_ROOT ${NUKE_DDIMAGE_LIBRARY} DIRECTORY)
message(STATUS "Found NDK: ${NUKE_ROOT}")

add_library(Nuke::NDK SHARED IMPORTED)

if (UNIX AND NOT APPLE)
    set_target_properties(Nuke::NDK
        PROPERTIES
            IMPORTED_LOCATION ${NUKE_ROOT}/libDDImage.so
            INTERFACE_INCLUDE_DIRECTORIES ${NUKE_ROOT}/include
            INTERFACE_LINK_LIBRARIES ${NUKE_ROOT}/libDDImage.so
            INTERFACE_COMPILE_FEATURES cxx_std_14
    )
elseif (APPLE)
    set_target_properties(Nuke::NDK
    PROPERTIES
        IMPORTED_LOCATION ${NUKE_ROOT}/libDDImage.dylib
        INTERFACE_INCLUDE_DIRECTORIES ${NUKE_ROOT}/include
        INTERFACE_LINK_LIBRARIES ${NUKE_ROOT}/libDDImage.dylib
        INTERFACE_COMPILE_FEATURES cxx_std_14
    )
else()
    set_target_properties(Nuke::NDK
        PROPERTIES
            IMPORTED_LOCATION ${NUKE_ROOT}/DDImage.dll
            IMPORTED_IMPLIB ${NUKE_ROOT}/DDImage.lib
            INTERFACE_INCLUDE_DIRECTORIES ${NUKE_ROOT}/include
            INTERFACE_COMPILE_FEATURES cxx_std_14
    )
endif()

# Add FDK library for latest versions of Nuke
if(${NUKE_VERSION_MAJOR} GREATER_EQUAL 14)
    add_library(Nuke::FDK SHARED IMPORTED)

    if(APPLE)
        set_target_properties(Nuke::FDK
            PROPERTIES
                IMPORTED_LOCATION ${NUKE_ROOT}/libFdkBase.dylib
                INTERFACE_INCLUDE_DIRECTORIES ${NUKE_ROOT}/include
                INTERFACE_LINK_LIBRARIES ${NUKE_ROOT}/libFdkBase.dylib
                INTERFACE_COMPILE_FEATURES cxx_std_14
        )
    elseif(UNIX)
        set_target_properties(Nuke::FDK
            PROPERTIES
                IMPORTED_LOCATION ${NUKE_ROOT}/libFdkBase.so
                INTERFACE_INCLUDE_DIRECTORIES ${NUKE_ROOT}/include
                INTERFACE_LINK_LIBRARIES ${NUKE_ROOT}/libFdkBase.so
                INTERFACE_COMPILE_FEATURES cxx_std_14
        )
    else()
        set_target_properties(Nuke::FDK
            PROPERTIES
                IMPORTED_LOCATION ${NUKE_ROOT}/FdkBase.dll
                IMPORTED_IMPLIB ${NUKE_ROOT}/FdkBase.lib
                INTERFACE_INCLUDE_DIRECTORIES ${NUKE_ROOT}/include
                INTERFACE_COMPILE_FEATURES cxx_std_14
        )
    endif()
endif()