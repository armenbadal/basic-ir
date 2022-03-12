
CMAKE_MINIMUM_REQUIRED(VERSION 3.18)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED TRUE)

PROJECT(basic-ir)

FIND_PACKAGE(LLVM REQUIRED CONFIG)

#add_definitions(-std=c++17)
#add_definitions(-gdwarf-2)

INCLUDE_DIRECTORIES(${LLVM_INCLUDE_DIRS})
ADD_DEFINITIONS(${LLVM_DEFINITIONS})

ADD_EXECUTABLE(${PROJECT_NAME}
	src/main.cxx
	src/compiler.cxx
	src/ast.cxx
	src/lexeme.cxx
	src/scanner.cxx
	src/parser.cxx
	src/astvisitor.cxx
	src/aslisp.cxx
	src/checker.cxx
	src/iremitter.cxx)

llvm_map_components_to_libnames(llvm_libs support core irreader linker)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${llvm_libs})

ADD_CUSTOM_COMMAND(
   OUTPUT ${CMAKE_BINARY_DIR}/basic_ir_lib.ll
   COMMAND clang -S -emit-llvm ${CMAKE_SOURCE_DIR}/src/basic_ir_lib.c)

ADD_CUSTOM_TARGET(${PROJECT_NAME}-lib ALL DEPENDS basic_ir_lib.ll)

ADD_DEPENDENCIES(${PROJECT_NAME} ${PROJECT_NAME}-lib)

#install(TARGETS basic-ir DESTINATION basic-ir)
#install(FILES ${CMAKE_BINARY_DIR}/basic_ir_lib.ll DESTINATION basic-ir)


