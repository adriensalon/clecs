add_executable(demo_core "main.cpp")
target_link_libraries(demo_core PRIVATE cl_ecs)
set_property(TARGET demo_core PROPERTY CXX_STANDARD 17)