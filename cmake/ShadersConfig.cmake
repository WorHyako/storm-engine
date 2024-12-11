cmake_minimum_required(VERSION 3.19)

### Copy Shader Effects
file(GLOB_RECURSE glob_techniques LIST_DIRECTORIES false RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/src" "src/techniques/**/*.fx")
set(technique_files "")
foreach(effect_file ${glob_techniques})
    #    message("${CMAKE_CURRENT_SOURCE_DIR}/src/${effect_file} -> ${CMAKE_CURRENT_BINARY_DIR}/bin/resource/${effect_file}")
    add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/bin/resource/${effect_file}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/src/${effect_file}
            ${CMAKE_CURRENT_BINARY_DIR}/bin/resource/${effect_file}
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/${effect_file}
    )
    list(APPEND technique_files ${CMAKE_CURRENT_BINARY_DIR}/bin/resource/${effect_file})
endforeach()
add_custom_target(engine_techniques DEPENDS ${technique_files})
###