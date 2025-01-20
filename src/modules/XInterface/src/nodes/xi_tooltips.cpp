#include "xi_tooltips.h"
#include "../xinterface.h"
#include "../str_utils.h"

#include "Filesystem/Config/Config.hpp"

using namespace Storm::Filesystem;
using namespace Storm::Math;

CXI_ToolTip::CXI_ToolTip(VXSERVICE *pPicService, VSTRSERVICE *pStrService, XYPOINT &pntScrSize)
    : m_pntScreenSize(pntScrSize)
{
    m_pPicService = pPicService;
    m_pStrService = pStrService;

    m_rs = XINTERFACE::GetRenderService();
    if (!m_rs)
    {
        throw std::runtime_error("No service: dx9render");
    }

    m_nTextureID = -1;
    m_pV = nullptr;
    m_pI = nullptr;
    m_nSquareQ = 0;
    m_dwBackColor = ARGB(255, 128, 128, 128);

    m_nFontID = -1;

    m_bDisableDraw = true;
    m_fTurnOnDelay = 2.f;
    m_fCurTimeLeft = 2.f;
    m_nMouseX = 0;
    m_nMouseY = 0;
}

CXI_ToolTip::~CXI_ToolTip()
{
    ReleaseAll();
}

void CXI_ToolTip::Draw()
{
    if (m_bDisableDraw)
        return;
    if (m_sText.empty())
        return; // there is nothing

    if (m_nSquareQ > 0)
    {
        m_rs->TextureSet(0, m_nTextureID);
        m_rs->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, m_nSquareQ * 4, m_nSquareQ * 2, m_pI, D3DFMT_INDEX16, m_pV,
                                     sizeof(XI_ONETEX_VERTEX), "iVideo");
    }

    const auto nX = (m_rPos.left + m_rPos.right) / 2;
    auto nY = m_rPos.top + m_pntTextOffset.y;
    for (int32_t n = 0; n < m_aSubText.size(); n++)
    {
        m_rs->ExtPrint(m_nFontID, m_dwFontColor, 0, PR_ALIGN_CENTER, true, m_fFontScale, m_pntScreenSize.x,
                       m_pntScreenSize.y, nX, nY, "%s", m_aSubText[n].c_str());
        nY += static_cast<int32_t>(m_rs->CharHeight(m_nFontID) * m_fFontScale);
    }
}

void CXI_ToolTip::SetByFormatString(XYRECT &rectOwner, const std::string& pDefIni, const char *pFmtStr)
{
    if (!pFmtStr)
        return;
    char tokenID[256];
    char tokenString[2048];
    char pcToolTipType[128];
    int32_t n;

    m_rActiveZone = rectOwner;

    sprintf_s(pcToolTipType, sizeof(pcToolTipType), "ToolTip");
    // TODO: figure out why this was done:
    int32_t m_nMaxStrWidth = -1; //~!~

    for (auto *pcParam = (char *)pFmtStr; pcParam && pcParam[0];)
    {
        if (nullptr == CXI_UTILS::StringGetTokenID(pcParam, tokenID, sizeof(tokenID)))
            break;
        const auto nTokenCode = CXI_UTILS::StringGetTokenCode(tokenID);
        if (CXI_UTILS::StringGetTokenString(pcParam, tokenString, sizeof(tokenString)))
        {
            const char *pStr = tokenString;
            switch (nTokenCode)
            {
            case InterfaceToken_text:
                if (tokenString[0] == '#')
                    m_sText = &tokenString[1];
                else
                    m_sText = m_pStrService->GetString(m_pStrService->GetStringNum(tokenString));
                break;
            case InterfaceToken_class:
                sprintf_s(pcToolTipType, sizeof(pcToolTipType), "%s", tokenString);
                break;
            case InterfaceToken_width:
                m_nMaxStrWidth = CXI_UTILS::StringGetInt(pStr);
            }
        }
    }

    //
    m_nFontID = -1;
    m_fFontScale = 1.f;
    m_dwFontColor = 0xFFFFFFFF;
    m_dwBackColor = ARGB(255, 128, 128, 128);
    m_nLeftSideWidth = m_nRightSideWidth = 0;
    m_uvBackLeft.left = m_uvBackRight.left = m_uvBackMiddle.left = 0.f;
    m_uvBackLeft.top = m_uvBackRight.top = m_uvBackMiddle.top = 0.f;
    m_uvBackLeft.right = m_uvBackRight.right = m_uvBackMiddle.right = 1.f;
    m_uvBackLeft.bottom = m_uvBackRight.bottom = m_uvBackMiddle.bottom = 1.f;
    m_pntTextOffset.x = 6;
    m_pntTextOffset.y = 4;
    m_sGroupName = "";
    m_nPicIndex_Left = m_nPicIndex_Right = m_nPicIndex_Middle = -1;

    if (pDefIni.empty())
    {
        auto config = Config::Load(pDefIni);
        std::ignore = config.SelectSection(pcToolTipType);

        auto font_id = config.Get<std::string>("font_id", {});
        if (!font_id.empty()) {
            m_nFontID = m_rs->LoadFont(font_id);
        }
        m_fFontScale = static_cast<float>(config.Get<double>("font_scale", m_fFontScale));

        auto font_color_opt = config.Get<Types::Vector4<std::int64_t>>("font_color");
        if (font_color_opt.has_value()) {
            m_dwFontColor = ARGB(font_color_opt->x, font_color_opt->y, font_color_opt->z, font_color_opt->w);
        }

        if (m_nMaxStrWidth <= 0) {
            m_nMaxStrWidth = static_cast<int>(config.Get<std::int64_t>("str_width", m_pntScreenSize.x));
        }
        auto text_offset_opt = config.Get<Types::Vector2<std::int64_t>>("str_offset");
        if (text_offset_opt) {
            m_pntTextOffset = *text_offset_opt;
        }

        auto back_color_opt = config.Get<Types::Vector4<std::int64_t>>("back_color");
        if (back_color_opt.has_value()) {
            m_dwBackColor = ARGB(back_color_opt->x, back_color_opt->y, back_color_opt->z, back_color_opt->w);
        }

        m_nLeftSideWidth = static_cast<int>(config.Get<std::int64_t>("back_leftwidth", m_nLeftSideWidth));
        m_nRightSideWidth = static_cast<int>(config.Get<std::int64_t>("back_rightwidth", m_nRightSideWidth));
        auto group_name = config.Get<std::string>("back_imagegroup", {});
        if (!group_name.empty())
        {
            m_sGroupName = group_name;
            m_nTextureID = m_pPicService->GetTextureID(m_sGroupName.c_str());
            const auto back_image_left_opt = config.Get<std::string>("back_imageleft");
            if (back_image_left_opt.has_value()) {
                m_nPicIndex_Left = m_pPicService->GetImageNum(m_sGroupName.c_str(), back_image_left_opt->c_str());
                m_pPicService->GetTexturePos(m_nPicIndex_Left, m_uvBackLeft);
            }
            const auto back_image_right_opt = config.Get<std::string>("back_imageright");
            if (back_image_right_opt.has_value()) {
                m_nPicIndex_Right = m_pPicService->GetImageNum(m_sGroupName.c_str(), back_image_right_opt->c_str());
                m_pPicService->GetTexturePos(m_nPicIndex_Right, m_uvBackRight);
            }
            const auto back_image_middle_opt = config.Get<std::string>("back_imagemiddle");
            if (back_image_middle_opt.has_value()) {
                m_nPicIndex_Middle = m_pPicService->GetImageNum(m_sGroupName.c_str(), back_image_middle_opt->c_str());
                m_pPicService->GetTexturePos(m_nPicIndex_Middle, m_uvBackMiddle);
            }
        }
        m_fTurnOnDelay = static_cast<float>(config.Get<double>("turnondelay", m_fTurnOnDelay));
        m_nXRectangleOffset = static_cast<int>(config.Get<std::int64_t>("horzcursoroffset", 0));
        m_nYRectangleOffsetUp = static_cast<int>(config.Get<std::int64_t>("vertupcursoroffset", 0));
        m_nYRectangleOffsetDown = static_cast<int>(config.Get<std::int64_t>("vertdowncursoroffset", 0));
    }
    if (m_nMaxStrWidth <= 0)
        m_nMaxStrWidth = m_pntScreenSize.x;

    //
    CXI_UTILS::SplitStringByWidth(m_sText.c_str(), m_nFontID, m_fFontScale, m_nMaxStrWidth, m_aSubText);
    m_nUseWidth = 0;
    for (n = 0; n < m_aSubText.size(); n++)
    {
        // m_aSubText[n].TrimLeft();
        // m_aSubText[n].TrimRight();
        TOREMOVE::trim(m_aSubText[n]);
        TOREMOVE::rtrim(m_aSubText[n]);

        const auto nW = m_rs->StringWidth((char *)m_aSubText[n].c_str(), m_nFontID, m_fFontScale, 0);
        if (nW > m_nUseWidth)
            m_nUseWidth = nW;
    }
    m_nUseWidth += m_pntTextOffset.x * 2;
    m_nUseHeight =
        m_aSubText.size() * static_cast<int32_t>(m_rs->CharHeight(m_nFontID) * m_fFontScale) + 2 * m_pntTextOffset.x;

    m_nSquareQ = 3;
    CreateIndexBuffer();
    CreateVertexBuffer();
    UpdateIndexBuffer();

    ReplaceRectangle(rectOwner.right, rectOwner.bottom);
}

void CXI_ToolTip::MousePos(float fDeltaTime, int32_t nX, int32_t nY)
{
    if (m_nMouseX != nX || m_nMouseY != nY || nX < m_rActiveZone.left || nX > m_rActiveZone.right ||
        nY < m_rActiveZone.top || nY > m_rActiveZone.bottom)
    {
        m_nMouseX = nX;
        m_nMouseY = nY;
        m_bDisableDraw = true;
        m_fCurTimeLeft = m_fTurnOnDelay;
        return;
    }

    if (m_fCurTimeLeft >= 0.f)
    {
        m_fCurTimeLeft -= fDeltaTime;
        if (m_fCurTimeLeft <= 0.f)
        {
            m_bDisableDraw = false;
            ReplaceRectangle(nX, nY);
        }
    }
}

void CXI_ToolTip::ReleaseAll()
{
    PICTURE_TEXTURE_RELEASE(m_pPicService, m_sGroupName.c_str(), m_nTextureID);
    STORM_DELETE(m_pV);
    STORM_DELETE(m_pI);
    m_nSquareQ = 0;
    m_bDisableDraw = true;
    FONT_RELEASE(m_rs, m_nFontID);
}

void CXI_ToolTip::CreateIndexBuffer()
{
    if (m_nSquareQ > 0)
    {
        m_pI = new uint16_t[m_nSquareQ * 6];
        Assert(m_pI);
    }
}

void CXI_ToolTip::CreateVertexBuffer()
{
    if (m_nSquareQ > 0)
    {
        m_pV = new XI_ONETEX_VERTEX[m_nSquareQ * 4];
        Assert(m_pV);
    }
}

void CXI_ToolTip::UpdateIndexBuffer() const
{
    if (!m_pI)
        return;
    for (int32_t n = 0; n < m_nSquareQ; n++)
    {
        m_pI[n * 6 + 0] = static_cast<uint16_t>(n * 4 + 0);
        m_pI[n * 6 + 1] = static_cast<uint16_t>(n * 4 + 1);
        m_pI[n * 6 + 2] = static_cast<uint16_t>(n * 4 + 2);

        m_pI[n * 6 + 3] = static_cast<uint16_t>(n * 4 + 1);
        m_pI[n * 6 + 4] = static_cast<uint16_t>(n * 4 + 3);
        m_pI[n * 6 + 5] = static_cast<uint16_t>(n * 4 + 2);
    }
}

void CXI_ToolTip::UpdateVertexBuffer()
{
    if (!m_pV)
        return;
    CXI_UTILS::WriteSquareToVertexBuffer(&m_pV[0], m_dwBackColor, m_uvBackMiddle, m_rPos.left + m_nLeftSideWidth,
                                         m_rPos.top, m_rPos.right - m_nRightSideWidth, m_rPos.bottom);
    CXI_UTILS::WriteSquareToVertexBuffer(&m_pV[4], m_dwBackColor, m_uvBackLeft, m_rPos.left, m_rPos.top,
                                         m_rPos.left + m_nLeftSideWidth, m_rPos.bottom);
    CXI_UTILS::WriteSquareToVertexBuffer(&m_pV[8], m_dwBackColor, m_uvBackRight, m_rPos.right - m_nRightSideWidth,
                                         m_rPos.top, m_rPos.right, m_rPos.bottom);
}

void CXI_ToolTip::ReplaceRectangle(int32_t x, int32_t y)
{
    auto top = y + m_nYRectangleOffsetUp;
    auto bottom = y + m_nYRectangleOffsetDown;
    if (top > m_rActiveZone.top)
        top = m_rActiveZone.top;
    if (bottom < m_rActiveZone.bottom)
        bottom = m_rActiveZone.bottom;

    // can we display info below the control?
    if (m_pntScreenSize.y - bottom >= m_nUseHeight)
    {
        m_rPos.top = bottom;
        m_rPos.bottom = m_rPos.top + m_nUseHeight;
    }
    else
    {
        if (top >= m_nUseHeight) // fits at the top
        {
            m_rPos.bottom = top;
            m_rPos.top = m_rPos.bottom - m_nUseHeight;
        }
        else
        {
            // put below anyway
            m_rPos.top = bottom;
            m_rPos.bottom = m_rPos.top + m_nUseHeight;
        }
    }

    // horizontal coordinates
    m_rPos.right = x + m_nUseWidth + m_nXRectangleOffset;
    if (m_rPos.right > m_pntScreenSize.x)
        m_rPos.right = m_pntScreenSize.x;
    m_rPos.left = m_rPos.right - m_nUseWidth;

    UpdateVertexBuffer();
}
