add_library(denoiser SHARED denoiser.cpp)
set_target_properties(denoiser PROPERTIES PREFIX "")
target_include_directories(denoiser PUBLIC ${OPENIMAGEDENOISE_INCLUDE_DIR} ${Nuke_INCLUDE_DIR})
target_link_libraries(denoiser PUBLIC ${Nuke_LIBRARIES} ${OPENIMAGEDENOISE_LIBRARY})

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