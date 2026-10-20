set(core "${CMAKE_CURRENT_SOURCE_DIR}/src/core" )
set(platform "${CMAKE_CURRENT_SOURCE_DIR}/src/platform" )
set(input "${CMAKE_CURRENT_SOURCE_DIR}/src/input" )
set(runtime "${CMAKE_CURRENT_SOURCE_DIR}/src/runtime" )

set(core_sources
	"${core}/logger/log.cpp"
)
set(runtime_sources
	"${runtime}/application.cpp"
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
    ${runtime_sources}
)
add_library(Engine::Engine ALIAS engine)

target_include_directories(engine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_features(engine PUBLIC cxx_std_20)
target_compile_definitions(engine 
    PRIVATE 
        ENGINE_BUILD
    PUBLIC
        SPDLOG_ACTIVE_LEVEL=$<IF:$<CONFIG:Debug>,SPDLOG_LEVEL_TRACE,SPDLOG_LEVEL_INFO>
)

if (MSVC)
    target_compile_options(engine PRIVATE /W4 /permissive-)
    # std::vector/std::string members in exported classes. Fine while engine and game
    # use the same compiler/CRT; revisit if you ever ship the DLL separately.
    target_compile_options(engine PUBLIC /wd4251)
else()
    target_compile_options(engine PRIVATE -Wall -Wextra)
endif()

target_link_libraries(engine 
        PUBLIC 
            spdlog::spdlog 
            glm::glm
        PRIVATE 
            glfw 
            header-only-libraries
)

target_precompile_headers(engine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")

set_target_properties(engine PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)