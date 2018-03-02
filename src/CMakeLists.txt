
cmake_minimum_required(VERSION 3.5)
project(basic-ir)
add_definitions(-std=c++14)
add_definitions(-gdwarf-2)
add_executable(basic-ir aslisp.cxx ast.cxx lexeme.cxx main.cxx parser.cxx scanner.cxx)

