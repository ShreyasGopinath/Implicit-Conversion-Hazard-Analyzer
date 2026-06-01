cmake_minimum_required(VERSION 3.22)

add_library(reporting_engine STATIC
    ReportingEngine.cpp
)

set_target_properties(reporting_engine PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(reporting_engine
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(reporting_engine
    PUBLIC
        fusion_engine
)
