find_package(DirectX11 REQUIRED)

if (DirectX11_FOUND)
    STORM_SETUP(
            TARGET_NAME renderer_dx11
            TYPE storm_module
            DEPENDENCIES core util renderer window glm ${SYSTEM_DEPS}
    )

    target_link_libraries(renderer_dx11 PUBLIC ${DirectX11_LIBRARIES})
    target_include_directories(renderer_dx11 PUBLIC ${DirectX11_INCLUDE_DIRS})
    target_compile_definitions(renderer_dx11 PUBLIC "-DSTORM_DIRECTX11_ENABLED")
endif ()