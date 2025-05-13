function(target_link_components target components_dir gen_dir)
    set(COMPONENTC_BUILD_DIR ${CMAKE_BINARY_DIR}/codegen/componentc)
    if(WIN32)
        set(COMPONENTC_EXECUTABLE ${COMPONENTC_BUILD_DIR}/${CMAKE_CFG_INTDIR}/componentc.exe)
    else()
        set(COMPONENTC_EXECUTABLE ${COMPONENTC_BUILD_DIR}/${CMAKE_CFG_INTDIR}/componentc)
    endif()
    set(STAMP_FILE ${gen_dir}/componentc.stamp)

    file(GLOB COMPONENT_FILES "${components_dir}/*.json")

    add_custom_command(
        OUTPUT ${STAMP_FILE}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${gen_dir}
        COMMAND ${COMPONENTC_EXECUTABLE} ${components_dir} ${gen_dir}
        COMMAND ${CMAKE_COMMAND} -E touch ${STAMP_FILE}
        DEPENDS ${COMPONENT_FILES} ${COMPONENTC_EXECUTABLE}
        COMMENT "Running componentc to generate components"
    )

    add_custom_target(generate_components_${target} DEPENDS ${STAMP_FILE})
    add_dependencies(generate_components_${target} componentc)
    add_dependencies(${target} generate_components_${target})
    target_include_directories(${target} PRIVATE ${gen_dir})
endfunction()

function(target_link_systems target systems_dir gen_dir)
    set(SYSTEMC_BUILD_DIR ${CMAKE_BINARY_DIR}/codegen/systemc)
    if(WIN32)
        set(SYSTEMC_EXECUTABLE ${SYSTEMC_BUILD_DIR}/${CMAKE_CFG_INTDIR}/systemc.exe)
    else()
        set(SYSTEMC_EXECUTABLE ${SYSTEMC_BUILD_DIR}/${CMAKE_CFG_INTDIR}/systemc)
    endif()
    set(STAMP_FILE ${gen_dir}/systemc.stamp)

    file(GLOB SYSTEM_FILES "${systems_dir}/*.cl")

    add_custom_command(
        OUTPUT ${STAMP_FILE}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${gen_dir}
        COMMAND ${SYSTEMC_EXECUTABLE} ${systems_dir} ${gen_dir}
        COMMAND ${CMAKE_COMMAND} -E touch ${STAMP_FILE}
        DEPENDS ${SYSTEM_FILES} ${SYSTEMC_EXECUTABLE}
        COMMENT "Running systemc to generate systems"
    )

    add_custom_target(generate_systems_${target} DEPENDS ${STAMP_FILE})
    add_dependencies(generate_systems_${target} systemc)
    add_dependencies(${target} generate_systems_${target})
    target_include_directories(${target} PRIVATE ${gen_dir})
endfunction()
