add_executable(game 
    "src/main.cpp"
    "src/PlatformerGame.cpp"
)

target_link_libraries(game 
    PRIVATE 
        Engine::Engine
)

target_compile_features(game PRIVATE cxx_std_20)

if (MSVC)
    target_compile_options(game PRIVATE /W4 /permissive-)
else()
    target_compile_options(game PRIVATE -Wall -Wextra)
endif()

add_custom_command(TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:engine>
        $<TARGET_FILE_DIR:game>
)