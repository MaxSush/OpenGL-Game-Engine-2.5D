add_executable(gamebox
    "src/main.cpp"
    "src/PlatformerGame.cpp"
    "src/gameplay/Player.cpp"
    "src/gameplay/Enemy.cpp"
    "src/gameplay/Collision.cpp"
    "src/gameplay/Level.cpp"
    "src/gameplay/CameraFollow.cpp"
    "src/gameplay/Rules.cpp"
)

target_link_libraries(gamebox 
    PRIVATE 
        Engine::Engine
)

target_compile_features(gamebox PRIVATE cxx_std_20)

target_include_directories(gamebox
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

if (MSVC)
    target_compile_options(gamebox PRIVATE /W4 /permissive-)
else()
    target_compile_options(gamebox PRIVATE -Wall -Wextra)
endif()

add_custom_command(TARGET gamebox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:engine>
        $<TARGET_FILE_DIR:gamebox>
)