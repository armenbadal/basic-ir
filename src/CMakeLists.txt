
cmake_minimum_required(VERSION 3.5)
project(basic-ir)

find_package(LLVM REQUIRED CONFIG)

add_definitions(-std=c++14)
add_definitions(-gdwarf-2)
add_executable(basic-ir aslisp.cxx astvisitor.cxx typechecker.cxx ast.cxx lexeme.cxx main.cxx parser.cxx scanner.cxx)


include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(basic-ir
	main.cxx
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
