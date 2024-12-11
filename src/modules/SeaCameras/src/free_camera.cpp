#include "free_camera.h"
#include "save_load.h"
#include "dx9render.h"

namespace
{

constexpr float kSensitivity = 0.0001f;
constexpr float kSpeed = 0.005f;

} // namespace

FREE_CAMERA::FREE_CAMERA()
    : position_(0.f, 30.f, 250.f)
{}

FREE_CAMERA::~FREE_CAMERA() = default;

bool FREE_CAMERA::Init()
{
    if (AttributesPointer->HasAttribute("position"))
    {
        const auto *position_attr = AttributesPointer->GetAttributeClass("position");
        position_.x = position_attr->GetAttributeAsFloat("x", 0);
        position_.y = position_attr->GetAttributeAsFloat("y", 0);
        position_.z = position_attr->GetAttributeAsFloat("z", 0);
        cameraOverride_ = true;
    }

    return true;
}

bool FREE_CAMERA::CreateState(ENTITY_STATE_GEN *state_gen) const
{
    state_gen->SetState("vv", sizeof(position_), position_, sizeof(angle_), angle_);
    return true;
}

bool FREE_CAMERA::LoadState(ENTITY_STATE *state)
{
    state->Struct(sizeof(position_), reinterpret_cast<char *>(&position_));
    state->Struct(sizeof(angle_), reinterpret_cast<char *>(&angle_));
    return true;
}

void FREE_CAMERA::Execute(uint32_t real_delta)
{
    if (AttributesPointer->HasAttribute("Perspective") )
    {
        SetPerspective(AttributesPointer->GetAttributeAsFloat("Perspective"));
    }

    float perspective{};
    if (!cameraOverride_)
    {
        GetRenderer().GetCamera(position_, angle_, perspective);
    }
    cameraOverride_ = false;

    Move(real_delta);
}

void FREE_CAMERA::Move(uint32_t real_delta)
{
    CONTROL_STATE cs{};
    core.Controls->GetControlState("FreeCamera_Turn_H", cs);
    angle_.y += kSensitivity * cs.fValue * real_delta;
    core.Controls->GetControlState("FreeCamera_Turn_V", cs);
    angle_.x += kSensitivity * cs.fValue * real_delta;

    const auto c0 = cosf(angle_.y);
    const auto s0 = sinf(angle_.y);
    const auto c1 = cosf(angle_.x);
    const auto s1 = sinf(angle_.x);
    float speed = kSpeed * static_cast<float>(real_delta);

    if (core.Controls->GetAsyncKeyState(VK_SHIFT))
    {
        speed *= 4.0f;
    }
    if (core.Controls->GetAsyncKeyState(VK_CONTROL))
    {
        speed *= 8.0f;
    }

    core.Controls->GetControlState("FreeCamera_Forward", cs);
    if (cs.state == CST_ACTIVE)
    {
        position_ += speed * CVECTOR(s0 * c1, -s1, c0 * c1);
    }
    core.Controls->GetControlState("FreeCamera_Backward", cs);
    if (cs.state == CST_ACTIVE)
    {
        position_ -= speed * CVECTOR(s0 * c1, -s1, c0 * c1);
    }

    GetRenderer().SetCamera(position_, angle_, GetPerspective());
}

void FREE_CAMERA::Save(CSaveLoad *pSL)
{
    pSL->SaveVector(position_);
    pSL->SaveVector(angle_);
    pSL->SaveFloat(fPerspective);
}

void FREE_CAMERA::Load(CSaveLoad *pSL)
{
    position_ = pSL->LoadVector();
    angle_ = pSL->LoadVector();
    fPerspective = pSL->LoadFloat();
}
