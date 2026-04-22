cmake_minimum_required(VERSION 3.10)
project(TicTacToe)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Silence Apple's deprecation warnings for SFML headers
add_compile_options(-Wno-deprecated-declarations)

# Find SFML Installation on your Mac
find_package(SFML 2.5 COMPONENTS graphics window system network audio REQUIRED)

# Specify resources to bundle into the .app
set(RESOURCE_FILES "arial.ttf")
set_source_files_properties(${RESOURCE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

# Create the .app wrapper using tictactoe.cpp instead of main.cpp
add_executable(TicTacToe MACOSX_BUNDLE tictactoe.cpp ${RESOURCE_FILES})

# Link SFML libraries
target_link_libraries(TicTacToe sfml-graphics sfml-window sfml-system sfml-network sfml-audio)