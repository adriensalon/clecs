cmake_minimum_required(VERSION 3.24)
project(tool_systemc)

add_executable(systemc "main.cpp")
set_property(TARGET systemc PROPERTY CXX_STANDARD 17)
