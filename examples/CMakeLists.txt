aux_source_directory(. EXAMPLES)
add_executable(framebuffer ${EXAMPLES})
target_include_directories(framebuffer PRIVATE include/)
target_link_libraries(framebuffer PRIVATE assimp::assimp)
target_link_libraries(framebuffer PRIVATE glad::glad)
target_link_libraries(framebuffer PRIVATE glfw)
target_link_libraries(framebuffer PRIVATE OpenGL::GL)
target_link_libraries(framebuffer PRIVATE glm::glm-header-only)
add_custom_target(copy_example_resources ALL 
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/examples/resources
    $<TARGET_FILE_DIR:framebuffer>/resources
    COMMENT "Copying resources"
)