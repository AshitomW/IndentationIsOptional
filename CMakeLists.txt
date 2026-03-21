cmake_minimum_required(VERSION 3.20)
project(indentationisoptional VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall -Wextra -Wpedantic -Werror
        -Wno-unused-parameter
        -Wshadow -Wnon-virtual-dtor
        -Wold-style-cast -Wcast-align
        -Woverloaded-virtual -Wconversion
        -Wsign-conversion -Wnull-dereference
        -Wformat=2
    )
elseif(MSVC)
    add_compile_options(/W4 /WX /permissive-)
endif()

add_library(indentationisoptional_core STATIC
    src/utils/utils.cpp
    src/transpiler/transpiler.cpp
    src/checker/checker.cpp
    src/lsp/lsp.cpp
)

target_include_directories(indentationisoptional_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(indentationisoptional src/main.cpp)
target_link_libraries(indentationisoptional PRIVATE indentationisoptional_core)

option(INDENTATIONISOPTIONAL_BUILD_TESTS "Build unit tests" ON)
if(INDENTATIONISOPTIONAL_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

install(TARGETS indentationisoptional DESTINATION bin)
install(TARGETS indentationisoptional_core DESTINATION lib)
install(DIRECTORY include/iio DESTINATION include)