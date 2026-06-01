add_library(discovery STATIC
    ConversionClassifier.cpp
    ConversionDiscoveryEngine.cpp
)

set_target_properties(discovery PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(discovery
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(discovery
    PUBLIC
        parsing
)
