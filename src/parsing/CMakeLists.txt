add_library(parsing STATIC
    ParsingServiceImpl.cpp
    detail/ImplicitCastVisitor.cpp
)

set_target_properties(parsing PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

llvm_map_components_to_libnames(LLVM_LIBS support core option)

set(CLANG_LIBS
    clangAST
    clangASTMatchers
    clangAnalysis
    clangBasic
    clangDriver
    clangEdit
    clangFrontend
    clangLex
    clangParse
    clangSema
    clangSerialization
    clangTooling
)

target_include_directories(parsing
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src
        ${LLVM_INCLUDE_DIRS}
)

target_compile_definitions(parsing
    PUBLIC
        ${LLVM_DEFINITIONS}
)

target_link_libraries(parsing
    PUBLIC
        ${LLVM_LIBS}
        ${CLANG_LIBS}
)
