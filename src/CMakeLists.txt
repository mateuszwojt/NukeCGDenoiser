add_library(denoiser SHARED denoiser.cpp)
set_target_properties(denoiser PROPERTIES PREFIX "")
target_link_libraries(denoiser ${Nuke_LIBRARIES} ${OpenImageDenoise_LIBRARIES})

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