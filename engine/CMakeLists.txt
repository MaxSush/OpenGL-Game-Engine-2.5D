set(core "${CMAKE_CURRENT_SOURCE_DIR}/src/core" )
set(platform "${CMAKE_CURRENT_SOURCE_DIR}/src/platform" )
set(input "${CMAKE_CURRENT_SOURCE_DIR}/src/input" )

set(core_sources
	"${core}/logger/log.cpp"
)
set(platform_sources
	"${platform}/glfw/glfw_window.cpp"
	"${platform}/glfw/glfw_input_backend.cpp"
)
set(input_sources
	"${input}/input.cpp"
	"${input}/input_frame.cpp"
	"${input}/input_action_map.cpp"
	"${input}/input_context_stack.cpp"
	"${input}/input_event_queue.cpp"
	"${input}/physical_input_state.cpp"
	"${input}/input_action_resolver.cpp"
	"${input}/input_system.cpp"
)

add_library(engine SHARED 
    ${core_sources}
    ${platform_sources}
    ${input_sources}
)
add_library(Engine::Engine ALIAS engine)

target_include_directories(engine
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"

        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_features(engine PUBLIC cxx_std_20)
target_compile_definitions(engine PRIVATE ENGINE_BUILD)

target_link_libraries(engine 
        PUBLIC 
            spdlog::spdlog 
        PRIVATE 
            glfw 
            header-only-libraries
)

target_precompile_headers(engine PRIVATE "pch.h")

set_target_properties(engine PROPERTIES
    CXX_EXTENSIONS OFF
)