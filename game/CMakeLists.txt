add_executable(game 
    "src/main.cpp"
    "src/PlatformerGame.cpp"
)

target_link_libraries(game 
    PRIVATE 
        Engine::Engine
)

target_compile_features(game PRIVATE cxx_std_20)

add_custom_command(TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:engine>
        $<TARGET_FILE_DIR:game>
)