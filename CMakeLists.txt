cmake_minimum_required(VERSION 3.28)

project(PackageMan LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(CURL REQUIRED)
find_package(LibArchive REQUIRED)

include(FetchContent)
set(REPROC++ ON CACHE BOOL "" FORCE)
FetchContent_Declare(reproc
    GIT_REPOSITORY https://github.com/daandemeyer/reproc.git
    GIT_TAG 14.2.8
    SYSTEM
    FIND_PACKAGE_ARGS NAMES reproc
)
FetchContent_MakeAvailable(reproc)

set(CLI11_PRECOMPILED ON CACHE BOOL "" FORCE)
FetchContent_Declare(cli11
    GIT_REPOSITORY https://github.com/cliutils/cli11.git
    GIT_TAG v2.7.2
    SYSTEM
)
FetchContent_MakeAvailable(cli11)

set(PACKAGEMAN_COMPILE_OPTIONS
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

# Everything except main, shared by the app and the tests
add_library(PackageManCore STATIC
    src/config.hpp        src/config.cpp
    src/utils/string.hpp
    src/apkindex.hpp      src/apkindex.cpp
    src/filedownload.hpp  src/filedownload.cpp
    src/archive.hpp       src/archive.cpp
    src/repository.hpp    src/repository.cpp
    src/graph.hpp         src/graph.cpp
    src/tempfile.hpp
    src/render.hpp        src/render.cpp
)
target_include_directories(PackageManCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(PackageManCore PUBLIC CURL::libcurl LibArchive::LibArchive reproc++ CLI11::CLI11)
target_compile_options(PackageManCore PRIVATE ${PACKAGEMAN_COMPILE_OPTIONS})

add_executable(PackageMan src/main.cpp)
target_link_libraries(PackageMan PRIVATE PackageManCore)
target_compile_options(PackageMan PRIVATE ${PACKAGEMAN_COMPILE_OPTIONS})

enable_testing()
add_executable(PackageManTests tests/tests.cpp)
target_link_libraries(PackageManTests PRIVATE PackageManCore)
target_compile_options(PackageManTests PRIVATE ${PACKAGEMAN_COMPILE_OPTIONS})
target_compile_definitions(PackageManTests PRIVATE TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/data")
add_test(NAME PackageManTests COMMAND PackageManTests)
