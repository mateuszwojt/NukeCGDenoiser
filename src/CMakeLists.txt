# Nuke
find_package(Nuke REQUIRED)

# OpenImageDenoise
find_package(OpenImageDenoise REQUIRED)

add_library(NukeCGDenoiser SHARED 
  denoiser.cpp
)

target_link_libraries(NukeCGDenoiser PUBLIC 
  Nuke::NDK
  OpenImageDenoise::OpenImageDenoise
)

set_target_properties(NukeCGDenoiser PROPERTIES 
  PREFIX ""
  INSTALL_RPATH "$ORIGIN/../lib"
)

install(
  TARGETS
    NukeCGDenoiser
  DESTINATION
    plugins
  COMPONENT
    Nuke
)
