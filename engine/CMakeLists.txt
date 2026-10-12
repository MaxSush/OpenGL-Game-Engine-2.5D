set(core "${CMAKE_CURRENT_SOURCE_DIR}/core" )

set(core_sources
	"${core}/logger/logger.cpp"
)

add_library(engine SHARED 
    ${core_sources}
    "pch.cpp"
)

target_include_directories(engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(engine PUBLIC spdlog::spdlog)

target_compile_definitions(engine PRIVATE ENGINE_BUILD)

target_precompile_headers(engine PRIVATE "pch.h")

set_target_properties(engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/engine"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/engine"
)