add_executable(game "src/main.cpp")

target_link_libraries(game PRIVATE engine)

add_custom_command(TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:engine>
        $<TARGET_FILE_DIR:game>
)