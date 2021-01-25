cmake_minimum_required(VERSION 3.9)

cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0068 NEW)

project(NukeCGDenoiser)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
  if(WIN32)
    # Was getting LNK4098: defaultlib 'MSCVRT' conflicts with use of other libs
    # It took many hours to figure out the -MT flag
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O2 -DNDEBUG -MT")
  else()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O3 -DNDEBUG -Wall")
  endif()
endif()

# Set install RPATH
set(CMAKE_INSTALL_RPATH "$ORIGIN/lib")
# Build with with install RPATH
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
# Add RPATHS outside of the build tree to install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
set(CMAKE_MACOSX_RPATH FALSE)

# CMake module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# /fPIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Nuke
find_package(Nuke REQUIRED)

# OpenImageDenoise
# find_package(OpenImageDenoise REQUIRED)
# if(NOT OPENIMAGEDENOISE_FOUND)
  message(STATUS "Compiling OIDN from source...")
  add_subdirectory(thirdparty/oidn)
# endif()

# OptiX
find_package(Optix REQUIRED)

if(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math")
endif()

# build plugin
add_library(denoiser SHARED denoiser.cpp)
set_target_properties(denoiser PROPERTIES PREFIX "")
target_link_libraries(denoiser PUBLIC ${Nuke_LIBRARIES})

if(MSVC)
  add_definitions(-DWIN32)
  add_definitions(-D_WIN32)
endif()


# Nuke home directory
if(WIN32 AND NOT CYGWIN)
  get_filename_component(Nuke_HOME_DIR "$ENV{HOMEPATH}/.nuke/" ABSOLUTE)
else()
  get_filename_component(Nuke_HOME_DIR "$ENV{HOME}/.nuke/" ABSOLUTE)
endif()

# Set default install prefix
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${Nuke_HOME_DIR}/nuke-denoiser" CACHE PATH "Install path prefix" FORCE)
  message("CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")
endif()

# Create init.py for $HOME/.nuke
if(EXISTS ${Nuke_HOME_DIR}/init.py)
  set(_pluginAddPath_FOUND "")
  file(READ ${Nuke_HOME_DIR}/init.py _nuke_INIT)
  if(_nuke_INIT)
    string(REGEX REPLACE "nuke.pluginAddPath\\(['\"]?(.+nuke-denoiser\\/?)['\"]?\\)" "\\0;\\1;" _mw_PLUGIN_PATH_LIST ${_nuke_INIT})
    list(LENGTH _mw_PLUGIN_PATH_LIST _mw_PLUGIN_PATH_LIST_LENGTH)
    if(${_mw_PLUGIN_PATH_LIST_LENGTH} GREATER 1)
      list(GET _mw_PLUGIN_PATH_LIST 1 _mw_PLUGIN_PATH)
      string(REGEX MATCH ${CMAKE_INSTALL_PREFIX} _mw_PLUGIN_PATH_MATCH ${_mw_PLUGIN_PATH})
      if(_mw_PLUGIN_PATH_MATCH)
        set(_pluginAddPath_FOUND "FOUND")
      endif()
    endif()
  endif()
  if(NOT _pluginAddPath_FOUND)
    set(_nuke_INIT "${_nuke_INIT}nuke.pluginAddPath('${CMAKE_INSTALL_PREFIX}')\n\n")
  endif()
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/init.py ${_nuke_INIT})
else()
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/init.py "import nuke\n\nnuke.pluginAddPath('${CMAKE_INSTALL_PREFIX}')\n\n")
endif()

install(
  TARGETS
    denoiser
  RUNTIME DESTINATION
    ${CMAKE_INSTALL_PREFIX}
  LIBRARY DESTINATION
    ${CMAKE_INSTALL_PREFIX}
  COMPONENT
    runtime
)
install(FILES menu.py DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/init.py DESTINATION ${Nuke_HOME_DIR})