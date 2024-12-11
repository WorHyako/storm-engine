cmake_minimum_required(VERSION 3.19)

# -------------- #
#   XInterface   #
# -------------- #
add_library(xinterface)
add_library(storm::xinterface ALIAS xinterface)

file(GLOB_RECURSE Sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
target_sources(xinterface
        PRIVATE ${Sources})

target_compile_definitions(xinterface
        PRIVATE NOMINMAX)

target_include_directories(xinterface
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include src)

target_link_libraries(xinterface
        PUBLIC
        ddraw
        amstrmid
        SDL2::SDL2$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:-static>
        storm::animation
        storm::model)
