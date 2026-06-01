cmake_minimum_required(VERSION 3.22)
project(ImplicitConversionHazardAnalyzer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(LLVM QUIET CONFIG)
find_package(Clang QUIET CONFIG)

if(LLVM_FOUND AND Clang_FOUND)
    message(STATUS "LLVM/Clang detected: enabling parsing/discovery modules")
    add_subdirectory(src/parsing)
    add_subdirectory(src/discovery)
    add_subdirectory(src/context)
else()
    message(WARNING "LLVM/Clang not found. Parsing-dependent modules will be unavailable.")
endif()

add_subdirectory(src/risk)
add_subdirectory(src/impact)

enable_testing()
add_subdirectory(tests)

add_subdirectory(examples)
