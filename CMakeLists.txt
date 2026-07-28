cmake_minimum_required(VERSION 4.2)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

project(basic-ir LANGUAGES CXX C)

find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(${PROJECT_NAME}
	src/main.cxx
	src/compiler.cxx
	src/lexeme.cxx
	src/scanner.cxx
	src/parser.cxx
	src/aslisp.cxx
	src/checker.cxx
	src/iremitter.cxx)

target_link_libraries(${PROJECT_NAME} PRIVATE LLVM)

add_custom_command(
   OUTPUT ${CMAKE_BINARY_DIR}/basic_ir_lib.ll
   COMMAND clang -S -emit-llvm ${CMAKE_SOURCE_DIR}/src/basic_ir_lib.c)

add_custom_target(${PROJECT_NAME}-lib ALL DEPENDS basic_ir_lib.ll)

add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}-lib)

# Tests
find_package(Catch2 3 REQUIRED QUIET)
if(Catch2_FOUND)
  add_executable(${PROJECT_NAME}-tests
    tests/aslisp_test.cxx
    tests/ast_test.cxx
    tests/lexeme_test.cxx
    tests/parser_test.cxx
    tests/scanner_test.cxx
    src/aslisp.cxx
    src/lexeme.cxx
    src/parser.cxx
    src/scanner.cxx)
  target_link_libraries(${PROJECT_NAME}-tests PRIVATE Catch2::Catch2WithMain)
  target_include_directories(${PROJECT_NAME}-tests PRIVATE src)
endif()


