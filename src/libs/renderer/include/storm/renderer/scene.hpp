#pragma once

#include "texture_handle.hpp"

#include <cstdint>
#include <vector>

namespace storm {

class Renderer;

struct Point {
    int32_t x;
    int32_t y;
};

class SceneNode {
  public:
    virtual ~SceneNode() = default;

    SceneNode &SetVisible(bool visible)
    {
        this->visible = visible;
        return *this;
    }

    bool IsVisible() const { return visible; }

  private:
    bool visible = true;

    friend storm::Renderer;
};

class Scene
{
  public:
    Scene &AddNode(SceneNode &node) {
        nodes_.push_back(&node);
        return *this;
    }

    uint32_t background = 0;

  public:
    std::vector<SceneNode*> nodes_;

    friend storm::Renderer;
};

class SpriteNode : public SceneNode {
  public:
    explicit SpriteNode(TextureHandle texture) : texture_(texture) {}

    SpriteNode &SetTexture(TextureHandle texture)
    {
        texture_ = texture;
        return *this;
    }

    [[nodiscard]] TextureHandle GetTexture() const { return texture_; }

    SpriteNode &SetPosition(const Point &position)
    {
        position_ = position;
        return *this;
    }

    [[nodiscard]] const Point &GetPosition() const { return position_; }

    SpriteNode &SetWidth(size_t width)
    {
        width_ = width;
        return *this;
    }

    SpriteNode &SetHeight(size_t height)
    {
        height_ = height;
        return *this;
    }

    [[nodiscard]] size_t GetWidth() const { return width_; }

    [[nodiscard]] size_t GetHeight() const { return height_; }

  private:
    TextureHandle texture_;
    Point position_;
    size_t width_;
    size_t height_;
};

} // namespace storm
