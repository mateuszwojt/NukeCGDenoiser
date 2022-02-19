if(NOT DEFINED OIDN_ROOT)
  if(WIN32)
    set(OIDN_ROOT "C:/oidn")
  elseif(APPLE)
    set(OIDN_ROOT "/opt/homebrew/Cellar/open-image-denoise")
  else()
    set(OIDN_ROOT "/usr/local/lib64")
  endif()
endif()

find_path(
  OpenImageDenoise_INCLUDE_DIR
  NAMES OpenImageDenoise/oidn.hpp
  PATHS ${OIDN_ROOT}/include/
  NO_DEFAULT_PATH
)

if(OpenImageDenoise_INCLUDE_DIR)
  include_directories(${OpenImageDenoise_INCLUDE_DIR})
endif()

find_library(
  OpenImageDenoise_LIBRARIES
  NAMES OpenImageDenoise
  PATHS ${OIDN_ROOT}/lib
  NO_DEFAULT_PATH
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenImageDenoise
  FOUND_VAR
    OpenImageDenoise_FOUND
  REQUIRED_VARS
    OIDN_ROOT
    OpenImageDenoise_INCLUDE_DIR
    OpenImageDenoise_LIBRARIES
)