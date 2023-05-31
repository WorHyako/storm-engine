set(RENDERER_DEPS "renderer")

if (DirectX11_FOUND)
    list(APPEND RENDERER_DEPS renderer_dx11)
endif ()

if (STORM_ENABLE_OPENGL)
    list(APPEND RENDERER_DEPS renderer_opengl)
endif ()

STORM_SETUP(
    TARGET_NAME core
    TYPE library
    DEPENDENCIES diagnostics math shared_headers steam_api fast_float ${SDL2_LIBRARIES} window ${RENDERER_DEPS}
)
