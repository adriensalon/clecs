cmake_minimum_required(VERSION 3.10)
project(cl_compute VERSION 1.0)

set(BUILD_SHARED_LIBS OFF)
find_package(OpenCL REQUIRED)

# codegen
include(cmake/codegen.cmake)
add_subdirectory(codegen)

# options
option(COMPUTE_BUILD_DEMO "Build demo executables" ON)
message("-- Build demo executables : ${COMPUTE_BUILD_DEMO}")

# lib
set(cl_compute_sources 
    "source/core/context.cpp"
    "source/core/device.cpp"
    "source/core/kernel.cpp"
    "source/ecs/registry.cpp"
)
add_library(cl_compute STATIC ${cl_compute_sources})
target_include_directories(cl_compute PUBLIC include)
target_link_libraries(cl_compute PUBLIC OpenCL::OpenCL)
set_property(TARGET cl_compute PROPERTY CXX_STANDARD 17)

# demo
if(COMPUTE_BUILD_DEMO)
    add_subdirectory(demo)
endif()
