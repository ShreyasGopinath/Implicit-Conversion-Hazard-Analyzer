cmake_minimum_required(VERSION 3.22)

add_executable(risk_engine_demo
    risk_engine_demo.cpp
)

target_link_libraries(risk_engine_demo
    PRIVATE
        risk_engine
)

target_include_directories(risk_engine_demo
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

add_executable(impact_graph_demo
    impact_graph_demo.cpp
)

target_link_libraries(impact_graph_demo
    PRIVATE
        impact_graph
)

target_include_directories(impact_graph_demo
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

add_executable(icha_integration_demo
    icha_integration_demo.cpp
)

target_link_libraries(icha_integration_demo
    PRIVATE
        impact_graph
        risk_engine
)

target_include_directories(icha_integration_demo
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

if(TARGET context_engine)
    add_executable(context_engine_demo
        context_engine_demo.cpp
    )

    target_link_libraries(context_engine_demo
        PRIVATE
            context_engine
    )

    target_include_directories(context_engine_demo
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
    )
endif()
