add_executable(demo_ecs "main.cpp")
target_link_libraries(demo_ecs PRIVATE cl_compute)
set_property(TARGET demo_ecs PROPERTY CXX_STANDARD 17)

target_link_components(demo_ecs 
    ${CMAKE_CURRENT_LIST_DIR}/component 
    ${CMAKE_CURRENT_LIST_DIR}/.gen)
    
target_link_systems(demo_ecs 
    ${CMAKE_CURRENT_LIST_DIR}/system 
    ${CMAKE_CURRENT_LIST_DIR}/.gen)
