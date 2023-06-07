add_library(denoiser SHARED denoiser.cpp)
target_include_directories (denoiser PUBLIC ${NUKE_INCLUDES})
set_target_properties(denoiser PROPERTIES PREFIX "")
target_link_libraries(denoiser ${NUKE_LIBRARIES} ${OpenImageDenoise_LIBRARIES})

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
