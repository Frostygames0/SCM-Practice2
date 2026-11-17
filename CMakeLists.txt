cmake_minimum_required(VERSION 3.19)

project(PackageMan LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(CURL REQUIRED)

add_executable(PackageMan 
    src/main.cpp
    src/config.hpp src/config.cpp
    src/string_utils.hpp
)

target_include_directories(PackageMan PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_include_directories(PackageMan PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
target_link_libraries(PackageMan PRIVATE CURL::libcurl)

target_compile_options(PackageMan PRIVATE
    # General
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /permissive->
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:-Wall -Wextra -Wpedantic -Wno-missing-field-initializers -Wshadow -Wnon-virtual-dtor>

    # Debug
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/Od /Ob0 /Zi /RTC1>
    $<$<AND:$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>,$<CONFIG:Debug>>:-O0 -g>

    # Release
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/O2 /Oi /Gy>
    $<$<AND:$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>,$<CONFIG:Release>>:-O2>
)

