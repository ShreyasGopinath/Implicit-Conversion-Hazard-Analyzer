enable_testing()

find_package(GTest CONFIG QUIET)

if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

if(TARGET parsing)
    add_executable(parsing_tests
        unit/parsing/ParsingServiceTests.cpp
    )

    target_link_libraries(parsing_tests
        PRIVATE
            parsing
            GTest::gtest_main
    )

    add_test(NAME parsing_tests COMMAND parsing_tests)
endif()

if(TARGET discovery)
    add_executable(discovery_tests
        unit/discovery/ConversionClassifierTests.cpp
    )

    target_link_libraries(discovery_tests
        PRIVATE
            discovery
            GTest::gtest_main
    )

    add_test(NAME discovery_tests COMMAND discovery_tests)
endif()

add_executable(risk_tests
    unit/risk/RiskEngineTests.cpp
    unit/risk/RiskEngineSpecificationTests.cpp
)

target_link_libraries(risk_tests
    PRIVATE
        risk_engine
        GTest::gtest_main
)

add_test(NAME risk_tests COMMAND risk_tests)

add_executable(impact_tests
    unit/impact/ConversionImpactGraphTests.cpp
)

target_link_libraries(impact_tests
    PRIVATE
        impact_graph
        GTest::gtest_main
)

add_test(NAME impact_tests COMMAND impact_tests)

if(TARGET context_engine)
    add_executable(context_tests
        unit/context/ContextIntelligenceTests.cpp
        unit/context/EnhancedContextIntelligenceTests.cpp
    )

    target_link_libraries(context_tests
        PRIVATE
            context_engine
            GTest::gtest_main
    )

    add_test(NAME context_tests COMMAND context_tests)
endif()
