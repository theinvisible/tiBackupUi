cmake_minimum_required(VERSION 3.16)

project(tiBackupUi VERSION 0.8.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

qt_standard_project_setup()

qt_add_executable(tiBackupUi
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    tibackupadd.cpp
    tibackupadd.h
    tibackupadd.ui
    tipreferences.cpp
    tipreferences.h
    tipreferences.ui
    tools/pbseditor.cpp
    tools/pbseditor.h
    tools/pbseditor.ui
    tools/pbsmanager.cpp
    tools/pbsmanager.h
    tools/pbsmanager.ui
    tools/scripteditor.cpp
    tools/scripteditor.h
    tools/scripteditor.ui
    tools/taskplaner.cpp
    tools/taskplaner.h
    tools/taskplaner.ui
    resui.qrc
)

target_include_directories(tiBackupUi PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# tiBackupLib: prefer the installed/exported CMake package (CI build-check installs it
# to a prefix and sets CMAKE_PREFIX_PATH). Fall back to building the sibling source tree
# in-tree, so local development needs no separate install step.
find_package(tiBackupLib CONFIG QUIET)
if(NOT tiBackupLib_FOUND)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../tiBackupLib/CMakeLists.txt")
        message(STATUS "tiBackupLib package not found - building sibling source tree ../tiBackupLib")
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../tiBackupLib ${CMAKE_BINARY_DIR}/tiBackupLib-build)
    else()
        message(FATAL_ERROR "tiBackupLib not found. Install the tibackuplib-dev package, set "
            "CMAKE_PREFIX_PATH to an install prefix, or place the tiBackupLib source tree next to this project.")
    endif()
endif()

target_compile_options(tiBackupUi PRIVATE
    $<$<CONFIG:Debug>:-pipe>
    $<$<CONFIG:Release>:-pipe;-O2>
)

target_link_libraries(tiBackupUi PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    tiBackupLib::tiBackupLib
)

install(TARGETS tiBackupUi
    RUNTIME DESTINATION /usr/bin
)
