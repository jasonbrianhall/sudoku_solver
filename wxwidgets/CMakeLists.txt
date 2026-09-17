cmake_minimum_required(VERSION 3.15)
project(SudokuSuite CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# wxWidgets ------------------------------------------------------------
# Linux:   apt install libwxgtk3.2-dev  (or libwxgtk3.0-gtk3-dev on older distros)
# Windows: vcpkg install wxwidgets   then configure with
#          -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake
find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

# Shared core library ----------------------------------------------------
add_library(sudoku_core STATIC
    sudoku.cpp
    generatepuzzle.cpp
    highscores.cpp
    debugqueue.cpp
    sudoku_wrapper.cpp
    grid_widget.cpp
)
target_include_directories(sudoku_core PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(sudoku_core PUBLIC ${wxWidgets_LIBRARIES})

if(WIN32)
    target_compile_definitions(sudoku_core PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()

# Solver app -----------------------------------------------------------
add_executable(sudoku_solver WIN32 solver_app.cpp)
target_link_libraries(sudoku_solver PRIVATE sudoku_core)

# Game app ---------------------------------------------------------------
add_executable(sudoku_game WIN32 game_app.cpp)
target_link_libraries(sudoku_game PRIVATE sudoku_core)

if(UNIX)
    target_link_libraries(sudoku_solver PRIVATE pthread)
    target_link_libraries(sudoku_game PRIVATE pthread)
endif()

# Optional app icon: drop an app.ico next to the built executable and
# both apps will pick it up at runtime (see SolverFrame/GameFrame ctors).
foreach(tgt sudoku_solver sudoku_game)
    if(EXISTS "${CMAKE_SOURCE_DIR}/app.ico")
        add_custom_command(TARGET ${tgt} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CMAKE_SOURCE_DIR}/app.ico" "$<TARGET_FILE_DIR:${tgt}>/app.ico")
    endif()
endforeach()
