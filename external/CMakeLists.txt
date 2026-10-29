
# Third-party dependencies

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

# spdlog
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE) 
set(SPDLOG_BUILD_PIC ON CACHE BOOL "" FORCE) 
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE) 
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE) 
set(SPDLOG_BUILD_BENCH OFF CACHE BOOL "" FORCE)
set(SPDLOG_COMPILED_LIB ON CACHE BOOL "" FORCE)
add_subdirectory(spdlog)

#glm
set(GLM_BUILD_LIBRARY OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(glm)

# imgui
add_subdirectory(imgui)

# EnTT (header-only ECS)
set(ENTT_BUILD_TESTING OFF CACHE BOOL "" FORCE)
add_subdirectory(entt)

# Expose header -only library 
add_library(header-only-libraries INTERFACE)

target_include_directories(header-only-libraries SYSTEM
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
