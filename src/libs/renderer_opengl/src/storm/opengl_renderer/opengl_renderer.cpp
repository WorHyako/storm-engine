#include "storm/opengl_renderer/opengl_renderer.hpp"
#include "core.h"
#include "sdl_window.hpp"

#include <SDL2/SDL.h>
#include <glbinding/gl33core/gl.h>
#include <glbinding/glbinding.h>

#include <iostream>

namespace storm
{

namespace
{

struct SpriteVertex
{
    float x{};
    float y{};
    float z{};
};

const char *kVertexShaderSource = "#version 330 core\n"
                                         "layout (location = 0) in vec3 aPos;\n"
                                         "void main()\n"
                                         "{\n"
                                         "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
                                         "}\0";

const char *kFragmentShaderSource = "#version 330 core\n"
                                           "out vec4 FragColor;\n"
                                           "\n"
                                           "void main()\n"
                                           "{\n"
                                           "    FragColor = vec4(0.0f, 0.0f, 1.0f, 1.0f);\n"
                                           "}";

} // namespace

class OpenGlRenderer::Impl
{
  public:
    explicit Impl(std::shared_ptr<OSWindow> window);

    std::shared_ptr<OSWindow> window_;

    gl::GLuint spriteVertexBuffer_{};
    gl::GLuint shaderProgram_{};

  private:
};

OpenGlRenderer::OpenGlRenderer(std::shared_ptr<OSWindow> window) : impl_(std::make_unique<Impl>(std::move(window)))
{
}

OpenGlRenderer::~OpenGlRenderer() = default;

void OpenGlRenderer::Init()
{
    auto &sdl_window = dynamic_cast<SDLWindow &>(*impl_->window_);
    const auto context = SDL_GL_CreateContext(sdl_window.SDLHandle());
    if (context == nullptr)
    {
        std::cerr << SDL_GetError() << '\n';
    }

    glbinding::initialize([](const char *fn) { return reinterpret_cast<void (*)()>(SDL_GL_GetProcAddress(fn)); });

    gl::GLuint vertex_shader = gl::glCreateShader(gl::GL_VERTEX_SHADER);
    gl::glShaderSource(vertex_shader, 1, &kVertexShaderSource, nullptr);
    gl::glCompileShader(vertex_shader);

    gl::GLuint fragment_shader = gl::glCreateShader(gl::GL_FRAGMENT_SHADER);
    gl::glShaderSource(fragment_shader, 1, &kFragmentShaderSource, nullptr);
    gl::glCompileShader(fragment_shader);

    impl_->shaderProgram_ = gl::glCreateProgram();
    gl::glAttachShader(impl_->shaderProgram_, vertex_shader);
    gl::glAttachShader(impl_->shaderProgram_, fragment_shader);
    gl::glLinkProgram(impl_->shaderProgram_);

    gl::glDeleteShader(vertex_shader);
    gl::glDeleteShader(fragment_shader);

    gl::glGenBuffers(1, &impl_->spriteVertexBuffer_);
}

void OpenGlRenderer::Render(const Scene &scene)
{
    ScreenSize screen_size{1024, 768};
    auto ini = fio->OpenIniFile(core.EngineIniFileName());
    if (ini) {
       screen_size.width = ini->GetInt(nullptr, "screen_x", 1024);
       screen_size.height = ini->GetInt(nullptr, "screen_y", 768);
    }

    gl::glViewport(0, 0, screen_size.width, screen_size.height);

    if (scene.background != 0) {
        const float red = static_cast<float>((scene.background & 0xFF0000) >> 16) / 255.f;
        const float green = static_cast<float>((scene.background & 0x00FF00) >> 8) / 255.f;
        const float blue = static_cast<float>((scene.background & 0x0000FF)) / 255.f;
        gl::glClearColor(red, green, blue, 1.0);
        gl::glClear(gl::ClearBufferMask::GL_COLOR_BUFFER_BIT | gl::ClearBufferMask::GL_DEPTH_BUFFER_BIT);
    }

    std::array<SpriteVertex, 4> vertices = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f,  0.5f, 0.0f
    };

    gl::glPolygonMode(gl::GL_FRONT_AND_BACK, gl::GL_FILL);
    gl::glUseProgram(impl_->shaderProgram_);

    ScreenSize canvas_size = core.GetScreenSize();
    const float half_screen_width = screen_size.width / 2.0f;
    const float half_screen_height = screen_size.height / 2.0f;

    for (const auto &node : scene.nodes_)
    {
        if (node->IsVisible())
        {
            if (const auto *sprite_node = dynamic_cast<SpriteNode *>(node); sprite_node != nullptr)
            {
                //                const TextureHandle texture_handle = sprite_node->GetTexture();
                //                if (texture_handle.IsValid())
                //                {
                //                    auto *texture =
                //                        static_cast<IDirect3DTexture9
                //                        *>(impl_->defaultTexturePool_.GetHandle(texture_handle));
                //                    impl_->device_->SetTexture(0, texture);
                //                }
                //                else {
                //                    impl_->device_->SetTexture(0, nullptr);
                //                }

                const auto &position = sprite_node->GetPosition();
                const size_t width = sprite_node->GetWidth();
                const size_t height = sprite_node->GetHeight();
                vertices[0].x = vertices[1].x = static_cast<float>(position.x - static_cast<int32_t>(width) / 2) / half_screen_width - 1.f;
                vertices[2].x = vertices[3].x = static_cast<float>(position.x + static_cast<int32_t>(width) / 2) / half_screen_width - 1.f;
                vertices[0].y = vertices[2].y = 1.f - (static_cast<float>(position.y - static_cast<int32_t>(height) / 2) / half_screen_height - 1.f);
                vertices[1].y = vertices[3].y = 1.f - (static_cast<float>(position.y + static_cast<int32_t>(height) / 2) / half_screen_height - 1.f);

                gl::glBindBuffer(gl::GL_ARRAY_BUFFER, impl_->spriteVertexBuffer_);
                gl::glBufferData(gl::GL_ARRAY_BUFFER, vertices.size() * 3 * sizeof(float), vertices.data(), gl::GL_DYNAMIC_DRAW);

                gl::glVertexAttribPointer(0, 3, gl::GL_FLOAT, gl::GL_FALSE, 3 * sizeof(float), nullptr);
                gl::glEnableVertexAttribArray(0);

                gl::glDrawArrays(gl::GL_TRIANGLE_STRIP, 0, 4);
            }
        }
    }
}

void OpenGlRenderer::Present()
{
    auto &sdl_window = dynamic_cast<SDLWindow &>(*impl_->window_);
    SDL_GL_SwapWindow(sdl_window.SDLHandle());
}

TextureHandle OpenGlRenderer::LoadTexture(const std::string_view &path)
{
    return TextureHandle(0);
}

OpenGlRenderer::Impl::Impl(std::shared_ptr<OSWindow> window) : window_(std::move(window))
{
}

} // namespace storm
