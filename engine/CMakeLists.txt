set(core "${CMAKE_CURRENT_SOURCE_DIR}/src/core" )
set(platform "${CMAKE_CURRENT_SOURCE_DIR}/src/platform" )

set(core_sources
	"${core}/logger/log.cpp"
)
set(platform_sources
	"${platform}/glfw/glfw_window.cpp"
)

add_library(engine SHARED 
    ${core_sources}
    ${platform_sources}
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
            glfw header-only-libraries
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