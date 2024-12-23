#include "texture_sequence.h"

#include "core.h"

#include "Filesystem/Config/Config.hpp"
#include "Filesystem/Constants/ConfigNames.hpp"

#define FILE_PATH "TextureSequence\\%s.tga"

#define TS_VERTEX_FRMT (D3DFVF_XYZRHW | D3DFVF_TEX2 | D3DFVF_TEXTUREFORMAT2)

struct TS_VERTEX
{
    CVECTOR pos;
    float w;
    float tu1, tv1;
    float tu2, tv2;
};

void GetRectFromNum(FRECT &texRect, int xq, int yq, int curNum, bool bHorzFlip, bool bVertFlip)
{
    auto y = curNum / xq;
    if (y >= yq)
        y = 0;
    auto x = curNum - y * xq;
    if (x >= xq)
        x = 0;

    const auto width = 1.f / xq;
    const auto height = 1.f / yq;

    if (bHorzFlip)
        texRect.left = width + (texRect.right = x * width);
    else
        texRect.right = width + (texRect.left = x * width);
    if (bVertFlip)
        texRect.top = height + (texRect.bottom = y * height);
    else
        texRect.bottom = height + (texRect.top = y * height);
}

TextureSequence::TextureSequence()
{
    m_pTexture = nullptr;
    m_AllTex = -1;
    m_bHorzFlip = false;
    m_bVertFlip = false;
}

TextureSequence::~TextureSequence()
{
    Release();
}

IDirect3DTexture9 *TextureSequence::Initialize(VDX9RENDER *pRS, const char *section, bool bCicled)
{
    m_bCicled = bCicled;
    if (pRS == nullptr || section == nullptr)
        return nullptr;
    m_pRS = pRS;

    // open ini file
    auto config = Storm::Filesystem::Config::Load(Storm::Filesystem::Constants::ConfigNames::texture_sequence());
    std::ignore = config.SelectSection(std::string(section));

    m_dwDeltaTime = config.Get<std::int64_t>("timeDelay", 128);
    if (m_dwDeltaTime == 0)
    {
        return nullptr;
    }
    m_texWidth = config.Get<std::int64_t>("width", 128);
    m_texHeight = config.Get<std::int64_t>("height", 128);
    m_xQuantity = config.Get<std::int64_t>("horzQ", 1);
    m_yQuantity = config.Get<std::int64_t>("vertQ", 1);
    m_maxCurNum = m_xQuantity * m_yQuantity;
    if (m_maxCurNum == 0)
    {
        return nullptr;
    }

    // load sequence texture
    const auto texture_name = config.Get<std::string>("TextureFile", "");
    m_AllTex = m_pRS->TextureCreate(texture_name.c_str());
    if (m_AllTex == -1)
    {
        return nullptr;
    }

    m_bHorzFlip = config.Get<std::int64_t>("flipH", 0) != 0;
    m_bVertFlip = config.Get<std::int64_t>("flipV", 0) != 0;

    m_pTexture = nullptr;
    // create output texture
    if (S_OK != m_pRS->CreateTexture(m_texWidth, m_texHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8,
                                     D3DPOOL_DEFAULT, &m_pTexture))
    {
        m_pTexture = nullptr;
        core.Trace("Can`t create texture");
        return nullptr;
    }

    // first render
    m_curNum = 0;
    m_dwCurDeltaTime = 0;
    // ToTextureRender(0.f);

    return m_pTexture;
}

//-----------------------------------------------------------------------------
// Name: FrameUpdate
// Desc: Performs per-frame updates
//-----------------------------------------------------------------------------
bool TextureSequence::FrameUpdate()
{
    m_dwCurDeltaTime += core.GetRDeltaTime();
    while (m_dwCurDeltaTime > m_dwDeltaTime)
    {
        m_dwCurDeltaTime -= m_dwDeltaTime;
        m_curNum++;
        if (m_curNum >= m_maxCurNum)
            m_curNum = 0;
    }

    ToTextureRender(static_cast<float>(m_dwCurDeltaTime) / static_cast<float>(m_dwDeltaTime));
    return true;
}

void TextureSequence::ToTextureRender(float blendValue) const
{
    const auto newTFactor = ARGB(static_cast<int32_t>(255.f * blendValue), static_cast<int32_t>(255.f * blendValue),
                                 static_cast<int32_t>(255.f * blendValue), static_cast<int32_t>(255.f * blendValue));

    // set texture as render target
    IDirect3DSurface9 *pRenderTarg = nullptr, *pOldRenderTarg = nullptr;
    if (m_pRS->GetRenderTarget(&pOldRenderTarg) == S_OK)
    {
        if (m_pRS->GetSurfaceLevel(m_pTexture, 0, &pRenderTarg) == S_OK)
        {
            IDirect3DSurface9 *pStencil;
            m_pRS->GetDepthStencilSurface(&pStencil);
            if (m_pRS->SetRenderTarget(pRenderTarg, nullptr) == S_OK)
            {
                CMatrix matw;
                m_pRS->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&matw);
                m_pRS->TextureSet(0, m_AllTex);
                m_pRS->TextureSet(1, m_AllTex);

                FRECT m_rectTex;
                TS_VERTEX v[4];
                for (auto i = 0; i < 4; i++)
                {
                    v[i].w = 0.5f;
                    v[i].pos.z = 1.f;
                }
                v[0].pos.x = 0;
                v[0].pos.y = 0;
                v[1].pos.x = 0;
                v[1].pos.y = static_cast<float>(m_texHeight);
                v[2].pos.x = static_cast<float>(m_texWidth);
                v[2].pos.y = 0;
                v[3].pos.x = static_cast<float>(m_texWidth);
                v[3].pos.y = static_cast<float>(m_texHeight);

                GetRectFromNum(m_rectTex, m_xQuantity, m_yQuantity, m_curNum, m_bHorzFlip, m_bVertFlip);
                v[0].tu1 = m_rectTex.left;
                v[0].tv1 = m_rectTex.bottom;
                v[1].tu1 = m_rectTex.left;
                v[1].tv1 = m_rectTex.top;
                v[2].tu1 = m_rectTex.right;
                v[2].tv1 = m_rectTex.bottom;
                v[3].tu1 = m_rectTex.right;
                v[3].tv1 = m_rectTex.top;

                GetRectFromNum(m_rectTex, m_xQuantity, m_yQuantity, m_curNum + 1, m_bHorzFlip, m_bVertFlip);
                v[0].tu2 = m_rectTex.left;
                v[0].tv2 = m_rectTex.bottom;
                v[1].tu2 = m_rectTex.left;
                v[1].tv2 = m_rectTex.top;
                v[2].tu2 = m_rectTex.right;
                v[2].tv2 = m_rectTex.bottom;
                v[3].tu2 = m_rectTex.right;
                v[3].tv2 = m_rectTex.top;

                // draw rectangle
                m_pRS->SetRenderState(D3DRS_TEXTUREFACTOR, newTFactor);
                m_pRS->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, TS_VERTEX_FRMT, 2, v, sizeof(TS_VERTEX), "TextureSequence");

                m_pRS->SetRenderTarget(pOldRenderTarg, pStencil);
            }
            pStencil->Release();
            pRenderTarg->Release();
        }
        pOldRenderTarg->Release();
    }
}

void TextureSequence::Release()
{
    if (m_pTexture != nullptr && m_pRS != nullptr)
    {
        m_pRS->Release(m_pTexture);
        m_pTexture = nullptr;
    }
    if (m_pRS != nullptr && m_AllTex != -1)
    {
        m_pRS->TextureRelease(m_AllTex);
        m_AllTex = -1;
    }
}

void TextureSequence::LostRender() const
{
    m_pRS->Release(m_pTexture);
}

void TextureSequence::RestoreRender()
{
    m_pRS->CreateTexture(m_texWidth, m_texHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
                         &m_pTexture);
}
