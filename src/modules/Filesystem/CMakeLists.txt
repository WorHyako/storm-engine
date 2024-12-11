cmake_minimum_required(VERSION 3.19)

# -------------- #
#   Filesystem   #
# -------------- #
add_library(filesystem)
add_library(storm::filesystem ALIAS filesystem)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
target_sources(filesystem
        PRIVATE
        ${Sources}
include/vma.hpp)

target_include_directories(filesystem
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include src)

target_link_libraries(filesystem
        PUBLIC
        nlohmann_json
        storm::utils
        tomlplusplus::tomlplusplus
        spdlog::spdlog_header_only)

# * remarks
#   файлы для настройки путей движка немножко потусуют в этом модуле,
#   но потом они перейдут в что-то типа `storm::struct_config`
