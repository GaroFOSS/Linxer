cmake_minimum_required(VERSION 4.1)
project(Linxer)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)

add_executable(Linxer
        main.cpp
        FileExplorerWindow.cpp
        FileUtils.cpp)

target_include_directories(Linxer PRIVATE ${GTKMM_INCLUDE_DIRS})
target_link_libraries(Linxer PRIVATE ${GTKMM_LIBRARIES})
target_compile_options(Linxer PRIVATE ${GTKMM_CFLAGS_OTHER})