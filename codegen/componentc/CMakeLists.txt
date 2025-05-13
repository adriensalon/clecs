cmake_minimum_required(VERSION 3.24)
project(tool_componentc)

add_executable(componentc "main.cpp")
target_include_directories(componentc PRIVATE "../../external/cereal/include")
set_property(TARGET componentc PROPERTY CXX_STANDARD 17)