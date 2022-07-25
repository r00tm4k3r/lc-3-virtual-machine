cmake_minimum_required(VERSION 3.10)

# set the project name
project(LC3 VERSION 0.0.1)

add_subdirectory(lc3_module)
#add the exe image
add_executable(lc3 main.cpp)

target_include_directories(
    lc3
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/lc3_module
    )

target_link_libraries(lc3 lc3_module)