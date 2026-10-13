cmake_minimum_required (VERSION 3.24)

project ("OpenGL-Game-Engine" 
		VERSION 0.1.0
		DESCRIPTION "A 2.5D OpenGL game engine"
		LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Include sub-projects.
add_subdirectory ("external")
add_subdirectory("engine")

if(PROJECT_IS_TOP_LEVEL)
    add_subdirectory(game)
endif()
