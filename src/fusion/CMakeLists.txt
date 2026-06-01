cmake_minimum_required(VERSION 3.22)

add_library(fusion_engine STATIC
    FusionEngine.cpp
    FindingSerializer.cpp
)

set_target_properties(fusion_engine PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(fusion_engine
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(fusion_engine
    PUBLIC
        impact_graph
        risk_engine
)
