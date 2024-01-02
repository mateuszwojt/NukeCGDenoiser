# Nuke
find_package(Nuke REQUIRED)

# OpenImageDenoise
find_package(OpenImageDenoise REQUIRED)

add_library(NukeDenoiser SHARED 
  denoiser.cpp
)

target_link_libraries(NukeDenoiser PUBLIC 
  Nuke::NDK
  OpenImageDenoise::OpenImageDenoise
)

set_target_properties(NukeDenoiser PROPERTIES 
  PREFIX ""
  INSTALL_RPATH "$ORIGIN/../lib"
)

install(
  TARGETS
    NukeDenoiser
  DESTINATION
    plugins
  COMPONENT
    Nuke
)
