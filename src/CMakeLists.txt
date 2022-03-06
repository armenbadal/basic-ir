
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
	main.cxx
	compiler.cxx
	ast.cxx
	lexeme.cxx
	scanner.cxx
	parser.cxx
	astvisitor.cxx
	aslisp.cxx
	checker.cxx
	iremitter.cxx)

llvm_map_components_to_libnames(llvm_libs support core irreader linker)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${llvm_libs})

ADD_CUSTOM_COMMAND(
   OUTPUT ${CMAKE_BINARY_DIR}/basic_ir_lib.ll
   COMMAND clang -S -emit-llvm ${CMAKE_SOURCE_DIR}/basic_ir_lib.c)

ADD_CUSTOM_TARGET(basic-ir-lib ALL DEPENDS basic_ir_lib.ll)

ADD_DEPENDENCIES(${PROJECT_NAME} basic-ir-lib)

#install(TARGETS basic-ir DESTINATION basic-ir)
#install(FILES ${CMAKE_BINARY_DIR}/basic_ir_lib.ll DESTINATION basic-ir)


