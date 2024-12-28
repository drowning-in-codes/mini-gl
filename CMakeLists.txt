cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(learn_gl VERSION 1.0 LANGUAGES C CXX)
aux_source_directory(src SOURCES)
add_executable(learn_gl app/main.cpp ${SOURCES})
find_package(OpenGL REQUIRED)
find_package(glad REQUIRED CONFIG)
find_package(glfw3 REQUIRED CONFIG)
find_package(assimp CONFIG REQUIRED)
target_link_libraries(learn_gl PRIVATE assimp::assimp)
target_include_directories(learn_gl PRIVATE src/include)
target_link_libraries(learn_gl PRIVATE glad::glad)
target_link_libraries(learn_gl PRIVATE glfw)
target_link_libraries(learn_gl PRIVATE OpenGL::GL)
find_package(glm CONFIG REQUIRED)
target_link_libraries(learn_gl PRIVATE glm::glm-header-only)
# message("${CMAKE_CXX_COMPILER_ID}")
add_custom_target(copy_resources ALL 
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/app/shaders
    $<TARGET_FILE_DIR:learn_gl>/shaders

    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/app/textures
    $<TARGET_FILE_DIR:learn_gl>/textures
    COMMENT "Copying resources"
)

# add_custom_command(TARGET learn_gl POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     ${CMAKE_SOURCE_DIR}/app/shaders
#     $<TARGET_FILE_DIR:learn_gl>/shaders
#     DEPENDS ${CMAKE_SOURCE_DIR}/app/shaders
# )

# add_custom_command(TARGET learn_gl POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     ${CMAKE_SOURCE_DIR}/app/textures
#     $<TARGET_FILE_DIR:learn_gl>/textures
# )

# file(GLOB_RECURSE TEXTURES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/app/textures/*)
# file(GLOB_RECURSE SHADERS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/app/shaders/*.)
# add_custom_target(outputMaterials ALL DEPENDS ${TEXTURES} COMMENT "Copying textures and shaders")
#  for closing console window
# if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows" AND CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
#     target_link_options(learn_gl PRIVATE  /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup)
# endif()