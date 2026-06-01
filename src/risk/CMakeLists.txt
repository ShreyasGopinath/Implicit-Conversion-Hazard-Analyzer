add_library(risk_engine STATIC
    RiskEngine.cpp
    RiskJsonFormatter.cpp
)

set_target_properties(risk_engine PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(risk_engine
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(risk_engine
    PUBLIC
)
