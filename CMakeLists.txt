cmake_minimum_required(VERSION 3.10)
project(TicTacToe)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Silence Mac warnings
if(APPLE)
    add_compile_options(-Wno-deprecated-declarations)
endif()

# Find SFML
find_package(SFML 2.5 COMPONENTS graphics window system network audio REQUIRED)

set(RESOURCE_FILES "arial.ttf")

if(APPLE)
    # Mac Build Instructions
    set_source_files_properties(${RESOURCE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    add_executable(TicTacToe MACOSX_BUNDLE tictactoe.cpp ${RESOURCE_FILES})
else()
    # Windows Build Instructions (WIN32 hides the console window)
    add_executable(TicTacToe WIN32 tictactoe.cpp ${RESOURCE_FILES})
endif()

target_link_libraries(TicTacToe sfml-graphics sfml-window sfml-system sfml-network sfml-audio)
