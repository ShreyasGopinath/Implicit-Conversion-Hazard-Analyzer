cmake_minimum_required(VERSION 3.22)

add_library(impact_graph
    ConversionImpactGraph.cpp
)

target_include_directories(impact_graph
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(impact_graph
    PUBLIC
        cxx_std_20
)
