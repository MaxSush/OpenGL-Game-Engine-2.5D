set(core "${CMAKE_CURRENT_SOURCE_DIR}/core" )
set(platform "${CMAKE_CURRENT_SOURCE_DIR}/platform" )

set(core_sources
	"${core}/logger/logger.cpp"
)
set(platform_sources
	"${platform}/glfw/glfw_window.cpp"
	"${platform}/platform_window.h"
)

add_library(engine SHARED 
    ${core_sources}
    ${platform_sources}
    "pch.cpp"
)

target_include_directories(engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(engine 
        PUBLIC spdlog::spdlog 
        PRIVATE glfw header-only-libraries
)

target_compile_definitions(engine PRIVATE ENGINE_BUILD)

target_precompile_headers(engine PRIVATE "pch.h")

set_target_properties(engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/engine"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/engine"
)