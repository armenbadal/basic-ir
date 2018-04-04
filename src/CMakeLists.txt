
cmake_minimum_required(VERSION 3.5)
project(basic-ir)

find_package(LLVM REQUIRED CONFIG)

add_definitions(-std=c++14)
add_definitions(-gdwarf-2)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(basic-ir
	main.cxx
	compiler.cxx
	ast.cxx
	lexeme.cxx
	scanner.cxx
	parser.cxx
	astvisitor.cxx
	aslisp.cxx
	typechecker.cxx
	iremitter.cxx)

llvm_map_components_to_libnames(llvm_libs support core irreader)

target_link_libraries(basic-ir ${llvm_libs})


add_custom_command(
   OUTPUT ${CMAKE_BINARY_DIR}/basic_ir_lib.ll
   COMMAND clang -S -emit-llvm ${CMAKE_SOURCE_DIR}/basic_ir_lib.c)

add_custom_target(basic-ir-lib ALL DEPENDS basic_ir_lib.ll)

add_dependencies(basic-ir basic-ir-lib)

#install(TARGETS basic-ir DESTINATION basic-ir)
#install(FILES ${CMAKE_BINARY_DIR}/basic_ir_lib.ll DESTINATION basic-ir)


