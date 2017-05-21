cmake_minimum_required(VERSION 3.5)
project(basic-ir)
add_definitions(-std=c++14)
add_executable(baic-ir main.cxx ast.cxx lexeme.cxx scanner.cxx parser.cxx)

