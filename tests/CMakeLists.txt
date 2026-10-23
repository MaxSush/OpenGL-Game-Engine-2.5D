set(ENGINE_SRC "${PROJECT_SOURCE_DIR}/engine/src")
set(GAME_SRC   "${PROJECT_SOURCE_DIR}/gamebox/src")

add_library(doctest_headers INTERFACE)

target_include_directories(doctest_headers SYSTEM INTERFACE 
	"${PROJECT_SOURCE_DIR}/external/doctest"
)

add_executable(engine_tests
    # test sources: add these one at a time while following the staged order in section 4
    test_main.cpp
    test_input_event_queue.cpp
    test_physical_input_state.cpp
    test_input_action_resolver.cpp
    test_collision.cpp
    test_player.cpp

    # production code under test, compiled directly (no GLFW, no GL, no DLL)
    "${ENGINE_SRC}/input/input_event_queue.cpp"
    "${ENGINE_SRC}/input/physical_input_state.cpp"
    "${ENGINE_SRC}/input/input_action_resolver.cpp"
    "${ENGINE_SRC}/input/input_action_map.cpp"
    "${GAME_SRC}/gameplay/Collision.cpp"
    "${GAME_SRC}/gameplay/Player.cpp"
)

target_compile_features(engine_tests PRIVATE cxx_std_20)

target_include_directories(engine_tests PRIVATE
    "${PROJECT_SOURCE_DIR}/engine/include"   # engine dll exported headers
    "${ENGINE_SRC}"                          # "input/physical_input_state.h" (internal headers)
    "${GAME_SRC}"                            # "gameplay/Player.h"
    "${CMAKE_CURRENT_SOURCE_DIR}"            # "test_helpers.h"
)

target_compile_definitions(engine_tests PRIVATE ENGINE_BUILD)

target_link_libraries(engine_tests 
    PRIVATE 
        glm::glm 
        doctest_headers
)

if (MSVC)
    target_compile_options(engine_tests PRIVATE /W4 /permissive- /wd4251)
else()
    target_compile_options(engine_tests PRIVATE -Wall -Wextra)
endif()

add_test(NAME engine_tests COMMAND engine_tests)    