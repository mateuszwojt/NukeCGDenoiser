add_library(denoiser SHARED denoiser.cpp)
set_target_properties(denoiser PROPERTIES PREFIX "")
include_directories(${OpenImageDenoise_INCLUDE_DIR} ${Nuke_INCLUDE_DIR})
target_link_libraries(denoiser ${Nuke_LIBRARIES} ${OpenImageDenoise_LIBRARY})

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