#include "rhi_bmfont.hpp"
#include "core.h"

#include <array>
#include <filesystem>
#include <fstream>

namespace storm::bmfont {

namespace {

constexpr size_t MAX_SYMBOLS = 512;
constexpr size_t SYM_VERTEXS = 6;

constexpr auto FONT_CHAR_FVF = ( VertexFVFBits::XYZ | VertexFVFBits::Color | VertexFVFBits::UV1 );

struct BMFONT_CHAR_VERTEX
{
    CVECTOR pos;
    float rhw;
    uint32_t color;
    float tu, tv, tg;
};

bool IsSignatureValid(const std::array<char, 4> &signature) {
    return signature[0] == 'B' && signature[1] == 'M' && signature[2] == 'F' && signature[3] == 3;
}

template<typename T>
T Read(std::ifstream &stream)
{
    T value{};
    stream.read(reinterpret_cast<char *>(&value), sizeof(value));
    return value;
}

std::string ReadString(std::ifstream &stream)
{
    std::string result{};
    result.resize(MAX_PATH);
    stream.get(reinterpret_cast<char *>(result.data()), result.size(), '\0');
    char next = stream.get();
    while (next != '\0') {
        size_t old_size = result.size();
        result.resize(old_size * 2);
        stream.get(reinterpret_cast<char *>(result.data() + old_size - 1), old_size, '\0');
        next = stream.get();
    }
    result.resize(strlen(result.c_str()));
    return result;
}

} // namespace

BmFont::BmFont(const std::string_view &file_path, RENDER &renderer) : renderer_(renderer)
{
    LoadFromFnt(std::string(file_path));

    vertexShader_ = device_->createShaderModule("font.vert");
    pixelShader_ = device_->createShaderModule("font.frag");
    textureSamler_ = device_->createTextureSampler();

    InitTextures();
    InitVertexBuffer();
}

BmFont::~BmFont()
{
    if (gradientTexture_ != -1)
    {
        renderer_.TextureRelease(gradientTexture_);
    }
}

std::optional<size_t> BmFont::Print(float x, float y, const std::string_view &text,
                                    const FontPrintOverrides &overrides)
{
    if (text.empty())
        return {};
    const float scale = scale_ * overrides.scale.value_or(1.f);
    const uint32_t color = overrides.color.value_or(0xFFFFFFFF);

    if (gradientTexture_ != -1)
    {
        renderer_.TechniqueExecuteStart("BmFontGradient");
    }
    else
    {
        renderer_.TechniqueExecuteStart("BmFont");
    }

    renderer_.TextureSet(0, textures_[0].textureHandle_);
    renderer_.TextureSet(1, gradientTexture_);
    renderer_.SetFVF(FONT_CHAR_FVF);
    renderer_.SetStreamSource(0, renderer_.GetVertexBuffer(vertexBuffer_), sizeof(BMFONT_CHAR_VERTEX));

    auto result = UpdateVertexBuffer(x, y, text, scale, color);
    renderer_.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    renderer_.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    renderer_.DrawPrimitive(D3DPT_TRIANGLELIST, 0, result.characters * 2);

    while(renderer_.TechniqueExecuteNext());

    return static_cast<size_t>(result.xoffset);
}

size_t BmFont::GetStringWidth(const std::string_view &text, const FontPrintOverrides &overrides) const
{
    if (text.empty())
        return 0;
    float xoffset = 0;
    const float scale = scale_ * overrides.scale.value_or(1.f);

    char32_t previous = '\0';

    for (size_t i = 0; i < text.size(); i += utf8::u8_inc(text.data() + i))
    {
        char32_t codepoint = utf8::Utf8ToCodepoint(text.data() + i);

        if (codepoint == 10 || codepoint == 13) {
            continue;
        }

        const BmCharacter *character = GetCharacter(codepoint);

        if (character == nullptr) {
            core.Trace("Unsupported codepoint: %d", codepoint);
            continue;
        }

        xoffset += static_cast<float>(GetKerning(previous, codepoint)) * scale;
        xoffset += static_cast<float>(character->xadvance) * scale;
        previous = codepoint;
    }

    return std::lround(xoffset);
}

size_t BmFont::GetHeight() const
{
    return std::lround(static_cast<double>(lineHeight_) * scale_ * lineHeightScaling_);
}

void BmFont::TempUnload()
{
    for (auto &texture : textures_) {
        renderer_.TextureRelease(texture.textureHandle_);
        texture.textureHandle_ = -1;
    }
}

void BmFont::RepeatInit()
{
    InitTextures();
}

BmFont &BmFont::SetScale(double scale)
{
    scale_ = scale;
    return *this;
}

BmFont &BmFont::SetLineScale(double scale)
{
    lineHeightScaling_ = scale;
    return *this;
}

BmFont &BmFont::SetGradient(int32_t gradient_texture)
{
    gradientTexture_ = gradient_texture;
    renderer_.TextureIncReference(gradientTexture_);
    return *this;
}

void BmFont::LoadFromFnt(const std::string &file_path)
{
    const std::filesystem::path path = file_path;
    const auto directory = path.parent_path();
    std::ifstream file(path, std::ios::binary);

    std::array<char, 4> signature{};
    file.read(signature.data(), signature.size());
    if (!IsSignatureValid(signature)) {
        throw std::runtime_error("Unsupported Bitmap Font file, signature check failed");
    }

    while(!file.eof()) {
        auto block_type = Read<int8_t>(file);
        auto block_size = Read<int32_t>(file);
        switch (block_type) {
        case 1: { // info
            file.ignore(2);
            auto bitfield = Read<int8_t>(file);
            if (!(bitfield & 0b0100'0000)) {
                throw std::runtime_error("Unsupported Bitmap Font file, not unicode");
            }
            file.ignore(block_size - 3);
            break;
        }
        case 2: { // common
            lineHeight_ = Read<int16_t>(file);
            base_ = Read<int16_t>(file);
            textureWidth_ = Read<int16_t>(file);
            textureHeight_ = Read<int16_t>(file);
            auto pages = Read<int16_t>(file);
            textures_.resize(pages);
            file.ignore(block_size - 10);
            break;
        }
        case 3: { // pages
            for (auto &texture : textures_) {
                texture.texturePath_ = (directory / std::filesystem::path(ReadString(file))).string();
            }
            break;
        }
        case 4: { // chars
            const auto count = block_size / 20;
            characters_.reserve(count);
            for (size_t i = 0; i < count; ++i) {
                characters_.emplace_back(BmCharacter{
                    .id = Read<char32_t>(file),
                    .x = Read<int16_t>(file),
                    .y = Read<int16_t>(file),
                    .width = Read<uint16_t>(file),
                    .height = Read<uint16_t>(file),
                    .xoffset = Read<int16_t>(file),
                    .yoffset = Read<int16_t>(file),
                    .xadvance = Read<int16_t>(file),
                    .page = Read<uint8_t>(file),
                    .channel = Read<uint8_t>(file),
                });
            }
            break;
        }
        case 5: { // kerning pairs
            const auto count = block_size / 10;
            kerning_.reserve(count);
            for (size_t i = 0; i < count; ++i) {
                kerning_.emplace_back(BmKerning{
                    .first = Read<char32_t>(file),
                    .second = Read<char32_t>(file),
                    .amount = Read<int16_t>(file),
                });
            }
            break;
        }
        default:
            file.ignore(block_size);
            break;
        }
    }
}

void BmFont::InitTextures()
{
    for (auto &texture : textures_) {
        if (texture.textureHandle_ == -1) {
            texture.textureHandle_ = renderer_.TextureCreate(texture.texturePath_.c_str());
        }
    }
}

void BmFont::InitVertexBuffer()
{
    vertexBuffer_ = renderer_.CreateVertexBuffer(sizeof(BMFONT_CHAR_VERTEX) * MAX_SYMBOLS * SYM_VERTEXS,
        RHI::MemoryPropertiesBits::HOST_VISIBLE_BIT | RHI::MemoryPropertiesBits::HOST_COHERENT_BIT);

    BMFONT_CHAR_VERTEX* vertices = reinterpret_cast<BMFONT_CHAR_VERTEX *>(device_->mapBufferMemory(vertexBuffer_.get(), 0, sizeof(BMFONT_CHAR_VERTEX) * MAX_SYMBOLS * SYM_VERTEXS));
    for (size_t codepoint = 0; codepoint < MAX_SYMBOLS * SYM_VERTEXS; codepoint++)
    {
        vertices[codepoint].pos.z = 0.5f;
    }
    device_->unmapBufferMemory(vertexBuffer_.get());
}

void BmFont::InitShader()
{
#ifdef _WIN32 // Effects
    fx_ = renderer_.GetEffectPointer("BmFont");
#endif
}

BmFont::UpdateVertexBufferResult BmFont::UpdateVertexBuffer(float x, float y, const std::string_view &text, float scale, uint32_t color)
{
    UpdateVertexBufferResult result{
        .xoffset = x,
    };

    auto *vertices = reinterpret_cast<BMFONT_CHAR_VERTEX *>(renderer_.LockVertexBuffer(vertexBuffer_));

    char32_t previous = '\0';

    for (size_t i = 0; i < text.size(); i += utf8::u8_inc(text.data() + i))
    {
        char32_t codepoint = utf8::Utf8ToCodepoint(text.data() + i);

        if (codepoint == 10 || codepoint == 13) {
            continue;
        }

        const BmCharacter *character = GetCharacter(codepoint);

        if (character == nullptr) {
            core.Trace("Unsupported codepoint: %d", codepoint);
            continue;
        }

        result.xoffset += static_cast<float>(GetKerning(previous, codepoint)) * scale;

        FLOAT_RECT pos {
            result.xoffset + (static_cast<float>(character->xoffset)) * scale,
            y + (static_cast<float>(character->yoffset)) * scale,
            result.xoffset + (static_cast<float>(character->xoffset + character->width)) * scale,
            y + (static_cast<float>(character->yoffset + character->height)) * scale,
        };

        BMFONT_CHAR_VERTEX *verts = vertices + result.characters * 6;
        ++result.characters;

        verts[0].pos.x = pos.x1;
        verts[1].pos.x = pos.x1;
        verts[2].pos.x = pos.x2;
        verts[3].pos.x = pos.x1;
        verts[4].pos.x = pos.x2;
        verts[5].pos.x = pos.x2;

        verts[0].pos.y = pos.y1;
        verts[1].pos.y = pos.y2;
        verts[2].pos.y = pos.y1;
        verts[3].pos.y = pos.y2;
        verts[4].pos.y = pos.y2;
        verts[5].pos.y = pos.y1;

        FLOAT_RECT uv {
            (static_cast<float>(character->x) + .5f) / textureWidth_,
            (static_cast<float>(character->y) + .5f) / textureHeight_,
            (static_cast<float>(character->x + character->width) + .5f) / textureWidth_,
            (static_cast<float>(character->y + character->height) + .5f) / textureHeight_,
        };

        verts[0].tu = uv.x1;
        verts[1].tu = uv.x1;
        verts[2].tu = uv.x2;
        verts[3].tu = uv.x1;
        verts[4].tu = uv.x2;
        verts[5].tu = uv.x2;

        verts[0].tv = uv.y1;
        verts[1].tv = uv.y2;
        verts[2].tv = uv.y1;
        verts[3].tv = uv.y2;
        verts[4].tv = uv.y2;
        verts[5].tv = uv.y1;

        float top = static_cast<float>(character->yoffset) / lineHeight_;
        float bottom = static_cast<float>(character->yoffset + character->height) / lineHeight_;

        verts[0].tg = top;
        verts[1].tg = bottom;
        verts[2].tg = top;
        verts[3].tg = bottom;
        verts[4].tg = bottom;
        verts[5].tg = top;

        for (size_t j = 0; j < SYM_VERTEXS; ++j) {
            verts[j].color = color;
            verts[j].rhw = scale;
        }

        result.xoffset += static_cast<float>(character->xadvance) * scale;

        previous = codepoint;
    }

    renderer_.UnLockVertexBuffer(vertexBuffer_);

    return result;
}

const BmCharacter *BmFont::GetCharacter(char32_t id) const
{
    auto found = std::lower_bound(std::begin(characters_), std::end(characters_), id, [] (const BmCharacter &character, char32_t id) {
        return character.id < id;
    });
    if (found->id == id) {
        return &*found;
    }
    return nullptr;
}

int16_t BmFont::GetKerning(char32_t first, char32_t second) const
{
    auto found = std::lower_bound(std::begin(kerning_), std::end(kerning_), first, [] (const BmKerning &kerning, char32_t id) {
        return kerning.first < id;
    });
    for(auto it = found; it < std::end(kerning_); ++it) {
        if (found->second == second) {
            return found->amount;
        }
    }
    return 0;
}

} // namespace storm::bmfont
