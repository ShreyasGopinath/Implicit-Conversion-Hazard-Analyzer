add_library(context_engine STATIC
    ContextIntelligenceEngine.cpp
    EnhancedContextIntelligenceEngine.cpp
)

set_target_properties(context_engine PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(context_engine
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

llvm_map_components_to_libnames(CONTEXT_LLVM_LIBS support core)

set(CONTEXT_CLANG_LIBS
    clangAST
    clangASTMatchers
    clangBasic
    clangFrontend
)

target_link_libraries(context_engine
    PUBLIC
        parsing
        ${CONTEXT_LLVM_LIBS}
        ${CONTEXT_CLANG_LIBS}
)
