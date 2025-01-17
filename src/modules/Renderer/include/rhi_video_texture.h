#pragma once
#include <renderer.h>

//-----------------------------------------------------------------------------
// Name: class VideoToTexture
// Desc: play video into texture
//-----------------------------------------------------------------------------
class CVideoTexture : public Entity
{
public:
    virtual RHI::TextureHandle Initialize(RENDER* pRS, const char* section, bool bCicled) = 0;
    virtual bool FrameUpdate() = 0;
    virtual void Release() = 0;
    RHI::TextureHandle m_pTexture;

    operator RHI::TextureHandle() const
    {
        return m_pTexture;
    }
};
