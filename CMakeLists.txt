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

#install(TARGETS basic-ir DESTINATION basic-ir)
#install(FILES ${CMAKE_BINARY_DIR}/basic_ir_lib.ll DESTINATION basic-ir)


