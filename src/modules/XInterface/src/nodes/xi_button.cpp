#include "xi_button.h"

#include "string_compare.hpp"

using namespace Storm::Filesystem;
using namespace Storm::Math;

CXI_BUTTON::CXI_BUTTON()
{
    m_rs = nullptr;
    m_idTex = -1;
    m_pTex = nullptr;

    fXShadow = 0.f;
    fYShadow = 0.f;

    nPressedDelay = 0;
    nMaxDelay = 100;

    m_nFontNum = -1;

    m_bClickable = true;
    m_nNodeType = NODETYPE_BUTTON;

    m_fBlindSpeed = -1.f;
}

CXI_BUTTON::~CXI_BUTTON()
{
    ReleaseAll();
}

void CXI_BUTTON::Draw(bool bSelected, uint32_t Delta_Time)
{
    if (nPressedDelay > 0)
        nPressedDelay--;

    if (m_bUse)
    {
        // Create rectangle
        XI_ONETEX_VERTEX vFace[4];
        XI_ONETEX_VERTEX vShadow[4];

        auto dwFaceColor = m_dwFaceColor;
        if (bSelected && m_fBlindSpeed > 0.f)
        {
            dwFaceColor = ColorInterpolate(m_dwDarkColor, m_dwLightColor, m_fCurBlind);
            if (m_bUpBlind)
                m_fCurBlind += m_fBlindSpeed * Delta_Time;
            else
                m_fCurBlind -= m_fBlindSpeed * Delta_Time;
            if (m_fCurBlind < 0.f)
            {
                m_fCurBlind = 0.f;
                m_bUpBlind = true;
            }
            if (m_fCurBlind > 1.f)
            {
                m_fCurBlind = 1.f;
                m_bUpBlind = false;
            }
        }

        for (auto i = 0; i < 4; i++)
        {
            vFace[i].color = dwFaceColor;
            vFace[i].pos.z = 1.f;
            vShadow[i].color = m_dwShadowColor;
            vShadow[i].pos.z = 1.f;
        }

        vFace[0].tu = vShadow[0].tu = m_tRect.left;
        vFace[0].tv = vShadow[0].tv = m_tRect.top;
        vFace[1].tu = vShadow[1].tu = m_tRect.right;
        vFace[1].tv = vShadow[1].tv = m_tRect.top;
        vFace[2].tu = vShadow[2].tu = m_tRect.left;
        vFace[2].tv = vShadow[2].tv = m_tRect.bottom;
        vFace[3].tu = vShadow[3].tu = m_tRect.right;
        vFace[3].tv = vShadow[3].tv = m_tRect.bottom;

        vFace[0].pos.x = static_cast<float>(m_rect.left);
        vFace[0].pos.y = static_cast<float>(m_rect.top);
        vFace[1].pos.x = static_cast<float>(m_rect.right);
        vFace[1].pos.y = static_cast<float>(m_rect.top);
        vFace[2].pos.x = static_cast<float>(m_rect.left);
        vFace[2].pos.y = static_cast<float>(m_rect.bottom);
        vFace[3].pos.x = static_cast<float>(m_rect.right);
        vFace[3].pos.y = static_cast<float>(m_rect.bottom);

        for (auto i = 0; i < 4; i++)
        {
            if (nPressedDelay > 0)
            {
                vFace[i].pos.x += fXDeltaPress;
                vFace[i].pos.y += fYDeltaPress;
                vShadow[i].pos.x = vFace[i].pos.x + fXShadowPress;
                vShadow[i].pos.y = vFace[i].pos.y + fYShadowPress;
            }
            else
            {
                vShadow[i].pos.x = vFace[i].pos.x + fXShadow;
                vShadow[i].pos.y = vFace[i].pos.y + fYShadow;
            }
        }

        if (m_idTex != -1)
            m_rs->TextureSet(0, m_idTex);
        else
            m_rs->SetTexture(0, m_pTex ? m_pTex->m_pTexture : nullptr);

        if (m_idTex >= 0 || m_pTex != nullptr)
        {
            m_rs->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, XI_ONETEX_FVF, 2, vShadow, sizeof(XI_ONETEX_VERTEX), "iShadow");
            if (m_bClickable && m_bSelected)
                m_rs->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, XI_ONETEX_FVF, 2, vFace, sizeof(XI_ONETEX_VERTEX),
                                      "iButton");
            else
            {
                m_rs->SetRenderState(D3DRS_TEXTUREFACTOR, m_argbDisableColor);
                m_rs->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, XI_ONETEX_FVF, 2, vFace, sizeof(XI_ONETEX_VERTEX),
                                      "iDisabledNode");
            }
        }

        if (m_idString != -1L)
            if (nPressedDelay > 0)
            {
                m_rs->ExtPrint(m_nFontNum, m_dwFontColor, 0, PR_ALIGN_CENTER, false, 1.f, m_screenSize.x,
                               m_screenSize.y, (m_rect.left + m_rect.right) / 2 + static_cast<int>(fXDeltaPress),
                               m_rect.top + m_dwStrOffset + static_cast<int>(fYDeltaPress), "%s",
                               pStringService->GetString(m_idString));
            }
            else
            {
                m_rs->ExtPrint(m_nFontNum, m_dwFontColor, 0, PR_ALIGN_CENTER, false, 1.f, m_screenSize.x,
                               m_screenSize.y, (m_rect.left + m_rect.right) / 2, m_rect.top + m_dwStrOffset, "%s",
                               pStringService->GetString(m_idString));
            }
    }
}

bool CXI_BUTTON::Init(const Config& node_config, const Config& def_config,
        VDX9RENDER *rs, XYRECT &hostRect, XYPOINT &ScreenSize)
{
    return CINODE::Init(node_config, def_config, rs, hostRect, ScreenSize);
}

void CXI_BUTTON::LoadIni(const Config& node_config, const Config& def_config) {
    std::pair<const Config&, const Config&> configs{node_config, def_config};
    m_nFontNum = m_rs->LoadFont(Config::GetOrGet<std::string>(configs, "font", {}));
    if (m_nFontNum == -1) {
        std::printf("Failed to load font");
    }
    auto face_color = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "faceColor", {255});
    m_dwFaceColor = ARGB(face_color.x, face_color.y, face_color.z, face_color.w);

    auto light_color = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "lightColor", {255});
    m_dwLightColor = ARGB(light_color.x, light_color.y, light_color.z, light_color.w);

    auto dark_color = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "darkColor", {255});
    m_dwDarkColor = ARGB(dark_color.x, dark_color.y, dark_color.z, dark_color.w);

    // blinking speed
    m_fBlindSpeed = static_cast<float>(Config::GetOrGet<double>(configs, "blindTimeSec", -1.0));
    if (m_fBlindSpeed <= 0.f)
        m_fBlindSpeed = 1.f;
    else
        m_fBlindSpeed = .001f / m_fBlindSpeed;
    m_fCurBlind = 1.f;
    m_bUpBlind = false;

    auto disable_color = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "disableColor", {255, 128, 128, 128});
    m_argbDisableColor = ARGB(disable_color.x, disable_color.y, disable_color.z, disable_color.w);

    auto shadow_color = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "shadowColor", {255, 0, 0, 0});
    m_dwShadowColor = ARGB(shadow_color.x, shadow_color.y, shadow_color.z, shadow_color.w);

    auto font_color = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "fontColor", {255});
    m_dwFontColor = ARGB(font_color.x, font_color.y, font_color.z, font_color.w);

    m_sGroupName = Config::GetOrGet<std::string>(configs, "group", {});
    if (!m_sGroupName.empty()) {
        m_idTex = pPictureService->GetTextureID(m_sGroupName.c_str());

        auto picture = Config::GetOrGet<std::string>(configs, "picture", {});
        if (!picture.empty()) {
            pPictureService->GetTexturePos(m_sGroupName.c_str(), picture.c_str(), m_tRect);
        }
    } else {
        auto video_texture = Config::GetOrGet<std::string>(configs, "videoTexture", {});
        if (!video_texture.empty()) {
            m_pTex = m_rs->GetVideoTexture(video_texture.c_str());
        }
        m_tRect.left = 0.f;
        m_tRect.top = 0.f;
        m_tRect.right = 1.f;
        m_tRect.bottom = 1.f;
    }

    // get offset button image in case pressed button
    auto press_picture_offset = Config::GetOrGet<Types::Vector2<std::int64_t>>(configs, "pressPictureOffset", {});
    fXDeltaPress = static_cast<float>(press_picture_offset.x);
    fYDeltaPress = static_cast<float>(press_picture_offset.y);

    // get offset button shadow in case pressed button
    auto shadow_offset = Config::GetOrGet<Types::Vector2<std::int64_t>>(configs, "shadowOffset", {});
    fXShadow = static_cast<float>(shadow_offset.x);
    fYShadow = static_cast<float>(shadow_offset.y);

    // get offset button shadow in case not pressed button
    auto press_shadow_offset = Config::GetOrGet<Types::Vector2<std::int64_t>>(configs, "pressShadowOffset", {});
    fXShadowPress = static_cast<float>(press_shadow_offset.x);
    fYShadowPress = static_cast<float>(press_shadow_offset.y);

    // get press delay
    nMaxDelay = Config::GetOrGet<std::int64_t>(configs, "pressDelay", 20);

    m_dwStrOffset = Config::GetOrGet<std::int64_t>(configs, "strOffset", 0);

    auto group = Config::GetOrGet<std::string>(configs, "group", {});
    m_idString = group.empty()
        ? -1
        : pStringService->GetStringNum(group.c_str());
}

void CXI_BUTTON::ReleaseAll()
{
    PICTURE_TEXTURE_RELEASE(pPictureService, m_sGroupName.c_str(), m_idTex);
    FONT_RELEASE(m_rs, m_nFontNum);
    VIDEOTEXTURE_RELEASE(m_rs, m_pTex);
}

int CXI_BUTTON::CommandExecute(int wActCode)
{
    if (m_bUse)
    {
        switch (wActCode)
        {
        case ACTION_ACTIVATE:
            nPressedDelay = nMaxDelay;
            break;
            // case ACTION_MOUSEDBLCLICK:
        case ACTION_MOUSECLICK:
            if (m_bClickable && m_bSelected)
                nPressedDelay = nMaxDelay;
            break;
        }
    }
    return -1;
}

bool CXI_BUTTON::IsClick(int buttonID, int32_t xPos, int32_t yPos)
{
    if (xPos >= m_rect.left && xPos <= m_rect.right && yPos >= m_rect.top && yPos <= m_rect.bottom && m_bClickable &&
        m_bSelected && m_bUse)
        return true;

    return false;
}

void CXI_BUTTON::ChangePosition(XYRECT &rNewPos)
{
    m_rect = rNewPos;
}

void CXI_BUTTON::SaveParametersToIni()
{
    char pcWriteParam[2048];

    auto pIni = fio->OpenIniFile(ptrOwner->m_sDialogFileName.c_str());
    if (!pIni)
    {
        core.Trace("Warning! Can`t open ini file name %s", ptrOwner->m_sDialogFileName.c_str());
        return;
    }

    // save position
    sprintf_s(pcWriteParam, sizeof(pcWriteParam), "%d,%d,%d,%d", m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
    pIni->WriteString(m_nodeName, "position", pcWriteParam);
}

void CXI_BUTTON::SetUsing(bool bUsing)
{
    m_bUse = bUsing;
    nPressedDelay = 0;
}

uint32_t CXI_BUTTON::MessageProc(int32_t msgcode, MESSAGE &message)
{
    switch (msgcode)
    {
    case 0: // change the position of the button
        m_rect.left = message.Long();
        m_rect.top = message.Long();
        m_rect.right = message.Long();
        m_rect.bottom = message.Long();
        GetAbsoluteRect(m_rect, message.Long());
        break;

    case 1: // change texture coordinates
        m_tRect.left = message.Float();
        m_tRect.top = message.Float();
        m_tRect.right = message.Float();
        m_tRect.bottom = message.Float();
        break;

    case 2: // change picture
    {
        const std::string &param = message.String();
        const auto len = param.size() + 1;
        if (len == 1)
            break;

        if (m_sGroupName.empty() || !storm::iEquals(m_sGroupName, param))
        {
            PICTURE_TEXTURE_RELEASE(pPictureService, m_sGroupName.c_str(), m_idTex);
            m_sGroupName.clear();

            m_sGroupName = std::string(param);
            m_idTex = pPictureService->GetTextureID(m_sGroupName.c_str());
        }

        const std::string &param2 = message.String();
        pPictureService->GetTexturePos(m_sGroupName.c_str(), param2.c_str(), m_tRect);
    }
    break;
    }

    return 0;
}
