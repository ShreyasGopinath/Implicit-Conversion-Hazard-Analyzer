cmake_minimum_required(VERSION 3.22)

# Main ICHA binary
if(LLVM_FOUND AND Clang_FOUND)
    add_executable(icha
        main.cpp
    )

    set_target_properties(icha PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
    )

    target_include_directories(icha
        PRIVATE
            ${PROJECT_SOURCE_DIR}/include
            ${LLVM_INCLUDE_DIRS}
            ${CLANG_INCLUDE_DIRS}
    )

    target_link_libraries(icha
        PRIVATE
            parsing
            discovery
            context_engine
            impact_graph
            risk_engine
            fusion_engine
            reporting_engine
            clangTooling
            clangBasic
            clangASTMatchers
    )

    # Install target
    install(TARGETS icha
        RUNTIME DESTINATION bin
    )

    message(STATUS "ICHA main binary will be built")
else()
    message(WARNING "LLVM/Clang not found. ICHA main binary will not be built.")
endif()
