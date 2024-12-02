#if (NOT WIN32)
#    set(SYSTEM_DEPS "${NATIVE_D3D9_LIBS}")
#endif()
#
#set(IMGUI_BINGINDS_DIR "${CMAKE_BINARY_DIR}/imgui")
#
#add_library(imgui_backend STATIC
#    "${IMGUI_BINGINDS_DIR}/imgui_impl_sdl2.cpp"
#    "${IMGUI_BINGINDS_DIR}/imgui_stdlib.cpp"
#)
#if (WIN32)
#    target_sources(imgui_backend PRIVATE "${IMGUI_BINGINDS_DIR}/imgui_impl_dx9.cpp")
#endif()
#
#target_include_directories(imgui_backend PUBLIC ${IMGUI_BINGINDS_DIR})
#target_link_libraries(imgui_backend PUBLIC imgui ${SDL2_LIBRARIES} ${SYSTEM_DEPS})
#
#STORM_SETUP(
#        TARGET_NAME editor
#        TYPE storm_module
#        DEPENDENCIES core imgui_backend
#        TEST_DEPENDENCIES catch2
#)
