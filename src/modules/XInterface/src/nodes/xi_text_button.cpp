#include "xi_text_button.h"

using namespace Storm::Filesystem;
using namespace Storm::Math;

CXI_TEXTBUTTON::CXI_TEXTBUTTON()
{
    m_rs = nullptr;

    m_idTex = -1;
    m_idShadowTex = -1;

    m_idVBuf = -1;
    m_idIBuf = -1;

    m_fXShadow = 0.f;
    m_fYShadow = 0.f;

    m_nPressedDelay = 0;
    m_nMaxDelay = 100;

    m_nFontNum = -1;
    m_fFontScale = 1.f;

    m_bClickable = true;
    m_bSelected = true;
    m_bMouseSelect = true;

    m_pTex = nullptr;
    m_nNodeType = NODETYPE_TEXTBUTTON;
    m_sString = nullptr;
    m_bVideoToBack = true;

    m_dwBackColor = ARGB(128, 0, 0, 0);
}

CXI_TEXTBUTTON::~CXI_TEXTBUTTON()
{
    ReleaseAll();
}

void CXI_TEXTBUTTON::Draw(bool bSelected, uint32_t Delta_Time)
{
    if (!m_bMakeActionInDeclick && m_nPressedDelay > 0)
        m_nPressedDelay--;

    if (m_bUse) {
        if (bSelected ^ m_bCurrentSelected)
        {
            auto *pVert = static_cast<XI_ONETEX_VERTEX *>(m_rs->LockVertexBuffer(m_idVBuf));
            if (pVert != nullptr)
            {
                FXYRECT texRect;
                m_bCurrentSelected = bSelected;

                if (m_idSelectMiddle != -1)
                {
                    if (bSelected)
                        pPictureService->GetTexturePos(m_idSelectMiddle, texRect);
                    else
                        pPictureService->GetTexturePos(m_idUnSelectMiddle, texRect);

                    pVert[4].tu = pVert[16].tu = pVert[28].tu = pVert[40].tu = texRect.left;
                    pVert[4].tv = pVert[16].tv = pVert[28].tv = pVert[40].tv = texRect.top;
                    pVert[5].tu = pVert[17].tu = pVert[29].tu = pVert[41].tu = texRect.right;
                    pVert[5].tv = pVert[17].tv = pVert[29].tv = pVert[41].tv = texRect.top;
                    pVert[6].tu = pVert[18].tu = pVert[30].tu = pVert[42].tu = texRect.left;
                    pVert[6].tv = pVert[18].tv = pVert[30].tv = pVert[42].tv = texRect.bottom;
                    pVert[7].tu = pVert[19].tu = pVert[31].tu = pVert[43].tu = texRect.right;
                    pVert[7].tv = pVert[19].tv = pVert[31].tv = pVert[43].tv = texRect.bottom;
                }

                if (m_idSelectLeft != -1)
                {
                    if (bSelected)
                        pPictureService->GetTexturePos(m_idSelectLeft, texRect);
                    else
                        pPictureService->GetTexturePos(m_idUnSelectLeft, texRect);

                    pVert[0].tu = pVert[12].tu = pVert[24].tu = pVert[36].tu = texRect.left;
                    pVert[0].tv = pVert[12].tv = pVert[24].tv = pVert[36].tv = texRect.top;
                    pVert[1].tu = pVert[13].tu = pVert[25].tu = pVert[37].tu = texRect.right;
                    pVert[1].tv = pVert[13].tv = pVert[25].tv = pVert[37].tv = texRect.top;
                    pVert[2].tu = pVert[14].tu = pVert[26].tu = pVert[38].tu = texRect.left;
                    pVert[2].tv = pVert[14].tv = pVert[26].tv = pVert[38].tv = texRect.bottom;
                    pVert[3].tu = pVert[15].tu = pVert[27].tu = pVert[39].tu = texRect.right;
                    pVert[3].tv = pVert[15].tv = pVert[27].tv = pVert[39].tv = texRect.bottom;
                }

                if (m_idSelectRight != -1 || m_idSelectLeft != -1)
                {
                    if (bSelected)
                    {
                        if (m_idSelectRight != -1)
                            pPictureService->GetTexturePos(m_idSelectRight, texRect);
                        else
                            pPictureService->GetTexturePos(TEXTURE_MODIFY_HORZFLIP, m_idSelectLeft, texRect);
                    }
                    else
                    {
                        if (m_idUnSelectRight != -1)
                            pPictureService->GetTexturePos(m_idUnSelectRight, texRect);
                        else
                            pPictureService->GetTexturePos(TEXTURE_MODIFY_HORZFLIP, m_idUnSelectLeft, texRect);
                    }

                    pVert[8].tu = pVert[20].tu = pVert[32].tu = pVert[44].tu = texRect.left;
                    pVert[8].tv = pVert[20].tv = pVert[32].tv = pVert[44].tv = texRect.top;
                    pVert[9].tu = pVert[21].tu = pVert[33].tu = pVert[45].tu = texRect.right;
                    pVert[9].tv = pVert[21].tv = pVert[33].tv = pVert[45].tv = texRect.top;
                    pVert[10].tu = pVert[22].tu = pVert[34].tu = pVert[46].tu = texRect.left;
                    pVert[10].tv = pVert[22].tv = pVert[34].tv = pVert[46].tv = texRect.bottom;
                    pVert[11].tu = pVert[23].tu = pVert[35].tu = pVert[47].tu = texRect.right;
                    pVert[11].tv = pVert[23].tv = pVert[35].tv = pVert[47].tv = texRect.bottom;
                }

                m_rs->UnLockVertexBuffer(m_idVBuf);
            }
        }

        // show shadow
        if (m_idShadowTex >= 0)
        {
            m_rs->TextureSet(0, m_idShadowTex);
            if (m_nPressedDelay > 0)
                m_rs->DrawPrimitive(D3DPT_TRIANGLESTRIP, m_idVBuf, sizeof(XI_ONETEX_VERTEX), 4 * 3 * 4, 2, "iIcon");
            else
                m_rs->DrawPrimitive(D3DPT_TRIANGLESTRIP, m_idVBuf, sizeof(XI_ONETEX_VERTEX), 4 * 3 * 4 + 4, 2, "iIcon");
        }
        else
        {
            m_rs->TextureSet(0, m_idTex);
            if (m_nPressedDelay > 0)
                m_rs->DrawBuffer(m_idVBuf, sizeof(XI_ONETEX_VERTEX), m_idIBuf, 4 * 3 * 3, 4 * 3, 0, m_nIndx, "iShadow");
            else
                m_rs->DrawBuffer(m_idVBuf, sizeof(XI_ONETEX_VERTEX), m_idIBuf, 4 * 3, 4 * 3, 0, m_nIndx, "iShadow");
        }

        if (m_bVideoToBack)
        {
            // show midle video fragment
            if (bSelected && m_pTex != nullptr) {
                m_rs->SetTexture(0, m_pTex->m_pTexture);
                if (m_nPressedDelay > 0)
                    m_rs->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, XI_ONETEX_FVF, 2, &m_v[4], sizeof(XI_ONETEX_VERTEX),
                                          "iVideo");
                else
                    m_rs->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, XI_ONETEX_FVF, 2, m_v, sizeof(XI_ONETEX_VERTEX),
                                          "iVideo");
            }
            else if (m_dwBackColor != 0) {
                m_rs->SetRenderState(D3DRS_TEXTUREFACTOR, m_dwBackColor);
                if (m_nPressedDelay > 0)
                    m_rs->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, XI_ONETEX_FVF, 2, &m_v[4], sizeof(XI_ONETEX_VERTEX),
                                          "iTFRectangle");
                else
                    m_rs->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, XI_ONETEX_FVF, 2, m_v, sizeof(XI_ONETEX_VERTEX),
                                          "iTFRectangle");
            }
        }

        // show button
        m_rs->TextureSet(0, m_idTex);
        if (m_nPressedDelay > 0) {
            m_rs->DrawBuffer(m_idVBuf, sizeof(XI_ONETEX_VERTEX), m_idIBuf, 4 * 3 * 2, 4 * 3, 0, m_nIndx, "iTextButton");
        } else {
            m_rs->DrawBuffer(m_idVBuf, sizeof(XI_ONETEX_VERTEX), m_idIBuf, 0, 4 * 3, 0, m_nIndx, "iTextButton");
        }

        if (!m_bVideoToBack) {
            // show midle video fragment
            if (bSelected && m_pTex != nullptr) {
                m_rs->SetTexture(0, m_pTex->m_pTexture);
                if (m_nPressedDelay > 0) {
                    m_rs->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, XI_ONETEX_FVF, 2, &m_v[4], sizeof(XI_ONETEX_VERTEX),
                                          "iVideo");
                } else {
                    m_rs->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, XI_ONETEX_FVF, 2, m_v, sizeof(XI_ONETEX_VERTEX),
                                          "iVideo");
                }
            }
        }

        if (m_idString != -1 || m_sString != nullptr) {
            if (m_nPressedDelay > 0) {
                m_rs->ExtPrint(m_nFontNum, m_dwPressedFontColor, 0, PR_ALIGN_CENTER, true, m_fFontScale, m_screenSize.x,
                               m_screenSize.y, (m_rect.left + m_rect.right) / 2 + static_cast<int>(m_fXDeltaPress),
                               m_rect.top + m_dwStrOffset + static_cast<int>(m_fYDeltaPress), "%s",
                               m_idString != -1 ? pStringService->GetString(m_idString) : m_sString);
            } else {
                if (m_bSelected) {
                    if(m_bCurrentSelected) {
                        m_rs->ExtPrint(m_nFontNum, m_dwSelectedFontColor, 0, PR_ALIGN_CENTER, true, m_fFontScale, m_screenSize.x,
                                       m_screenSize.y, (m_rect.left + m_rect.right) / 2, m_rect.top + m_dwStrOffset, "%s",
                                       m_idString != -1 ? pStringService->GetString(m_idString) : m_sString);
                    } else {
                        m_rs->ExtPrint(m_nFontNum, m_dwFontColor, 0, PR_ALIGN_CENTER, true, m_fFontScale, m_screenSize.x,
                                       m_screenSize.y, (m_rect.left + m_rect.right) / 2, m_rect.top + m_dwStrOffset, "%s",
                                       m_idString != -1 ? pStringService->GetString(m_idString) : m_sString);
                    }
                }
                else
                    m_rs->ExtPrint(m_nFontNum, m_dwUnselFontColor, 0, PR_ALIGN_CENTER, true, m_fFontScale,
                                   m_screenSize.x, m_screenSize.y, (m_rect.left + m_rect.right) / 2,
                                   m_rect.top + m_dwStrOffset, "%s",
                                   m_idString != -1 ? pStringService->GetString(m_idString) : m_sString);
            }
        }
    }
}

bool CXI_TEXTBUTTON::Init(const Config& node_config, const Config& def_config,
    VDX9RENDER *rs, XYRECT &hostRect, XYPOINT &ScreenSize) {
    return CINODE::Init(node_config, def_config, rs, hostRect, ScreenSize);
}

void CXI_TEXTBUTTON::LoadIni(const Config& node_config, const Config& def_config) {
    std::pair<const Config&, const Config&> configs{node_config, def_config};
    // video to back
    m_bVideoToBack = Config::GetOrGet<std::int64_t>(configs, "videoToBack", 1);

    // get back color for button
    auto back_color = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "backColor", {255});
    m_dwBackColor = ARGB(back_color.x, back_color.y, back_color.z, back_color.w);

    // get face color for not pressed button
    auto face_color = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "faceColor", {255});
    m_dwFaceColor = ARGB(face_color.x, face_color.y, face_color.z, face_color.w);

    // get face color for pressed button
    auto face_color_pressed = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "faceColorPressed", {255});
    m_dwPressedFaceColor = ARGB(face_color_pressed.x, face_color_pressed.y, face_color_pressed.z, face_color_pressed.w);

    // get shadow color
    auto shadow_color = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "shadowColor", {255, 0, 0, 0});
    m_dwShadowColor = ARGB(shadow_color.x, shadow_color.y, shadow_color.z, shadow_color.w);

    // get font color
    auto font_color = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "fontColor", {255});
    m_dwFontColor = ARGB(font_color.x, font_color.y, font_color.z, font_color.w);

    // get font color for pressed button
    auto font_color_pressed = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "fontColorPressed");
    m_dwPressedFontColor = font_color_pressed.has_value()
        ? ARGB(font_color_pressed->x, font_color_pressed->y, font_color_pressed->z, font_color_pressed->w)
        : m_dwFontColor;

    // get font color for selected button
    auto font_color_selected_opt = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "fontColorSelected");
    m_dwSelectedFontColor = font_color_selected_opt.has_value()
        ? ARGB(font_color_selected_opt->x, font_color_selected_opt->y, font_color_selected_opt->z, font_color_selected_opt->w)
        : m_dwFontColor;

    m_fFontScale = static_cast<float>(Config::GetOrGet<double>(configs, "fontScale", 1.0));

    // get deselected font color
    auto font_color_unselected = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "unselectableFontColor", {255, 128, 128, 128});
    m_dwUnselFontColor = ARGB(font_color_unselected.x, font_color_unselected.y, font_color_unselected.z, font_color_unselected.w);

    // get group name and get texture for this
    m_sGroupName = Config::GetOrGet<std::string>(configs, "group", {});
    m_idTex = m_sGroupName.empty()
        ? -1
        : pPictureService->GetTextureID(m_sGroupName.c_str());

    auto shadow_texture = Config::GetOrGet<std::string>(configs, "ShadowTexture", {});
    m_idShadowTex = shadow_texture.empty()
        ? -1
        : m_idShadowTex = m_rs->TextureCreate(shadow_texture.c_str());

    FXYRECT frectShadowUV;
    if (m_idShadowTex >= 0) {
        frectShadowUV = Config::GetOrGet<Types::Vector4<double>>(configs, "ShadowUV", {0.0, 0.0, 1.0, 1.0});
    }

    // get offset button image in case pressed button
    FXYPOINT fPos;
    fPos = Config::GetOrGet<Types::Vector2<std::int64_t>>(configs, "pressPictureOffset", {}).to<float>();
    m_fXDeltaPress = fPos.x;
    m_fYDeltaPress = fPos.y;

    // get offset button shadow in case pressed button
    fPos = Config::GetOrGet<Types::Vector2<std::int64_t>>(configs, "shadowOffset", {}).to<float>();
    m_fXShadow = fPos.x;
    m_fYShadow = fPos.y;

    // get offset button shadow in case not pressed button
    fPos = Config::GetOrGet<Types::Vector2<std::int64_t>>(configs, "pressShadowOffset", {}).to<float>();
    m_fXShadowPress = fPos.x;
    m_fYShadowPress = fPos.y;

    // get press delay
    m_nMaxDelay = Config::GetOrGet<std::int64_t>(configs, "pressDelay", 20);

    // get string parameters
    auto font = Config::GetOrGet<std::string>(configs, "font", {});
    if (!font.empty()) {
        m_nFontNum = m_rs->LoadFont(font);
        if (m_nFontNum == -1) {
            core.Trace("can not load font:'%s'", font.c_str());
        }
    }
    m_dwStrOffset = Config::GetOrGet<std::int64_t>(configs, "strOffset", 0);

    auto str = Config::GetOrGet<std::string>(configs, "string", {});
    m_idString = str.empty()
        ? -1
        : pStringService->GetStringNum(str.c_str());

    // get video fragment parameters
    auto mid_video = Config::GetOrGet<std::string>(configs, "midVideo", {});
    m_pTex = mid_video.empty()
        ? nullptr
        : m_rs->GetVideoTexture(mid_video.c_str());

    // do vertex and index buffer
    m_nIndx = 3 * 2 * 3;     // 3 rectangle * 2 triangle to rectangle * 3 vertex to triangle
    m_nVert = 4 * 3 * 2 * 2; // 4 vertex * 3 rectangle * (2=face&shadow) * (2=press&notpress)
    if (m_idShadowTex >= 0) {
        m_nVert += 8;
    }
    m_idIBuf = m_rs->CreateIndexBuffer(m_nIndx * 2);
    m_idVBuf = m_rs->CreateVertexBuffer(XI_ONETEX_FVF, m_nVert * sizeof(XI_ONETEX_VERTEX), D3DUSAGE_WRITEONLY);

    // Lock buffers for write
    auto *pVert = static_cast<XI_ONETEX_VERTEX *>(m_rs->LockVertexBuffer(m_idVBuf));
    auto *pIndx = static_cast<uint16_t *>(m_rs->LockIndexBuffer(m_idIBuf));
    if (pVert == nullptr || pIndx == nullptr)
        throw std::runtime_error("can not create the index&vertex buffers");

    // fill triangles buffer
    auto i = 0;
    for (auto tidx = 0; tidx < 3; tidx++) {
        pIndx[i + 0] = tidx * 4;
        pIndx[i + 1] = tidx * 4 + 1;
        pIndx[i + 2] = tidx * 4 + 2;
        pIndx[i + 3] = tidx * 4 + 2;
        pIndx[i + 4] = tidx * 4 + 1;
        pIndx[i + 5] = tidx * 4 + 3;
        i += 6;
    }

    // fill vertex buffer
    for (i = 0; i < m_nVert; i++)
        pVert[i].pos.z = 1.f;
    for (i = 0; i < 4 * 3; i++)
        pVert[i].color = m_dwFaceColor;
    for (; i < 4 * 3 * 2; i++)
        pVert[i].color = m_dwShadowColor;
    for (; i < 4 * 3 * 3; i++)
        pVert[i].color = m_dwPressedFaceColor;
    for (; i < 4 * 3 * 4; i++)
        pVert[i].color = m_dwShadowColor;

    if (m_idShadowTex >= 0) {
        m_fShadowScale = Config::GetOrGet<double>(configs, "shadowScale", 1.0);

        for (; i < m_nVert; i++) {
            pVert[i].color = m_dwShadowColor;
        }

        i = m_nVert - 8;
        pVert[i + 4].tu = pVert[i + 0].tu = frectShadowUV.left;
        pVert[i + 4].tv = pVert[i + 0].tv = frectShadowUV.top;
        pVert[i + 5].tu = pVert[i + 1].tu = frectShadowUV.left;
        pVert[i + 5].tv = pVert[i + 1].tv = frectShadowUV.bottom;
        pVert[i + 6].tu = pVert[i + 2].tu = frectShadowUV.right;
        pVert[i + 6].tv = pVert[i + 2].tv = frectShadowUV.top;
        pVert[i + 7].tu = pVert[i + 3].tu = frectShadowUV.right;
        pVert[i + 7].tv = pVert[i + 3].tv = frectShadowUV.bottom;

        const auto fhorzoffset = (m_rect.right - m_rect.left) * (m_fShadowScale - 1.f) * .5f;
        const auto fvertoffset = (m_rect.bottom - m_rect.top) * (m_fShadowScale - 1.f) * .5f;

        pVert[i + 0].pos.x = m_rect.left - fhorzoffset + m_fXShadow;
        pVert[i + 0].pos.y = m_rect.top - fvertoffset + m_fYShadow;
        pVert[i + 1].pos.x = m_rect.left - fhorzoffset + m_fXShadow;
        pVert[i + 1].pos.y = m_rect.bottom + fvertoffset + m_fYShadow;
        pVert[i + 2].pos.x = m_rect.right + fhorzoffset + m_fXShadow;
        pVert[i + 2].pos.y = m_rect.top - fvertoffset + m_fYShadow;
        pVert[i + 3].pos.x = m_rect.right + fhorzoffset + m_fXShadow;
        pVert[i + 3].pos.y = m_rect.bottom + fvertoffset + m_fYShadow;

        pVert[i + 4].pos.x = m_rect.left - fhorzoffset + m_fXDeltaPress + m_fXShadow;
        pVert[i + 4].pos.y = m_rect.top - fvertoffset + m_fYDeltaPress + m_fYShadow;
        pVert[i + 5].pos.x = m_rect.left - fhorzoffset + m_fXDeltaPress + m_fXShadow;
        pVert[i + 5].pos.y = m_rect.bottom + fvertoffset + m_fYDeltaPress + m_fYShadow;
        pVert[i + 6].pos.x = m_rect.right + fhorzoffset + m_fXDeltaPress + m_fXShadow;
        pVert[i + 6].pos.y = m_rect.top - fvertoffset + m_fYDeltaPress + m_fYShadow;
        pVert[i + 7].pos.x = m_rect.right + fhorzoffset + m_fXDeltaPress + m_fXShadow;
        pVert[i + 7].pos.y = m_rect.bottom + fvertoffset + m_fYDeltaPress + m_fYShadow;
    }

    FXYRECT texRect;
    XYRECT natureRect;
    // fill left side of button
    m_idUnSelectLeft = m_idSelectLeft = -1;
    auto selected_button_left = Config::GetOrGet<std::string>(configs, "selectButtonLeft", {});
    if (!selected_button_left.empty()) {
        m_idSelectLeft = pPictureService->GetImageNum(m_sGroupName.c_str(), selected_button_left.c_str());
    }
    auto button_left = Config::GetOrGet<std::string>(configs, "buttonLeft", {});
    if (!button_left.empty()) {
        m_idUnSelectLeft = pPictureService->GetImageNum(m_sGroupName.c_str(), button_left.c_str());
    }
    pPictureService->GetTexturePos(m_idUnSelectLeft, texRect);
    pPictureService->GetTexturePos(m_idUnSelectLeft, natureRect);
    auto fLeftMiddle = static_cast<float>(m_rect.left + natureRect.right - natureRect.left);
    pVert[0].tu = pVert[12].tu = pVert[24].tu = pVert[36].tu = texRect.left;
    pVert[0].tv = pVert[12].tv = pVert[24].tv = pVert[36].tv = texRect.top;
    pVert[1].tu = pVert[13].tu = pVert[25].tu = pVert[37].tu = texRect.right;
    pVert[1].tv = pVert[13].tv = pVert[25].tv = pVert[37].tv = texRect.top;
    pVert[2].tu = pVert[14].tu = pVert[26].tu = pVert[38].tu = texRect.left;
    pVert[2].tv = pVert[14].tv = pVert[26].tv = pVert[38].tv = texRect.bottom;
    pVert[3].tu = pVert[15].tu = pVert[27].tu = pVert[39].tu = texRect.right;
    pVert[3].tv = pVert[15].tv = pVert[27].tv = pVert[39].tv = texRect.bottom;
    pVert[0].pos.x = pVert[2].pos.x = static_cast<float>(m_rect.left);
    pVert[12].pos.x = pVert[14].pos.x = m_rect.left + m_fXShadow;
    pVert[24].pos.x = pVert[26].pos.x = m_rect.left + m_fXDeltaPress;
    pVert[36].pos.x = pVert[38].pos.x = m_rect.left + m_fXDeltaPress + m_fXShadowPress;
    pVert[0].pos.y = pVert[1].pos.y = static_cast<float>(m_rect.top);
    pVert[12].pos.y = pVert[13].pos.y = m_rect.top + m_fYShadow;
    pVert[24].pos.y = pVert[25].pos.y = m_rect.top + m_fYDeltaPress;
    pVert[36].pos.y = pVert[37].pos.y = m_rect.top + m_fYDeltaPress + m_fYShadowPress;
    pVert[1].pos.x = pVert[3].pos.x = fLeftMiddle;
    pVert[13].pos.x = pVert[15].pos.x = fLeftMiddle + m_fXShadow;
    pVert[25].pos.x = pVert[27].pos.x = fLeftMiddle + m_fXDeltaPress;
    pVert[37].pos.x = pVert[39].pos.x = fLeftMiddle + m_fXDeltaPress + m_fXShadowPress;
    pVert[2].pos.y = pVert[3].pos.y = static_cast<float>(m_rect.bottom);
    pVert[14].pos.y = pVert[15].pos.y = m_rect.bottom + m_fYShadow;
    pVert[26].pos.y = pVert[27].pos.y = m_rect.bottom + m_fYDeltaPress;
    pVert[38].pos.y = pVert[39].pos.y = m_rect.bottom + m_fYDeltaPress + m_fYShadowPress;
    // fill right side of button
    auto fRightMiddle = m_rect.right - (fLeftMiddle - m_rect.left);
    m_idUnSelectRight = m_idSelectRight = -1;
    auto button_right = Config::GetOrGet<std::string>(configs, "buttonRight", {});
    if (!button_right.empty()) {
        m_idUnSelectRight = pPictureService->GetImageNum(m_sGroupName.c_str(), button_right.c_str());
    }
    auto selected_button_right = Config::GetOrGet<std::string>(configs, "selectButtonRight", {});
    if (!selected_button_right.empty()) {
        m_idSelectRight = pPictureService->GetImageNum(m_sGroupName.c_str(), selected_button_right.c_str());
    }

    if (m_idUnSelectRight != -1) {
        pPictureService->GetTexturePos(m_idUnSelectRight, texRect);
        pPictureService->GetTexturePos(m_idUnSelectRight, natureRect);
    } else {
        pPictureService->GetTexturePos(TEXTURE_MODIFY_HORZFLIP, m_idUnSelectLeft, texRect);
        pPictureService->GetTexturePos(m_idUnSelectLeft, natureRect);
    }
    fRightMiddle = m_rect.right - static_cast<float>(natureRect.right - natureRect.left);
    pVert[8].tu = pVert[20].tu = pVert[32].tu = pVert[44].tu = texRect.left;
    pVert[8].tv = pVert[20].tv = pVert[32].tv = pVert[44].tv = texRect.top;
    pVert[9].tu = pVert[21].tu = pVert[33].tu = pVert[45].tu = texRect.right;
    pVert[9].tv = pVert[21].tv = pVert[33].tv = pVert[45].tv = texRect.top;
    pVert[10].tu = pVert[22].tu = pVert[34].tu = pVert[46].tu = texRect.left;
    pVert[10].tv = pVert[22].tv = pVert[34].tv = pVert[46].tv = texRect.bottom;
    pVert[11].tu = pVert[23].tu = pVert[35].tu = pVert[47].tu = texRect.right;
    pVert[11].tv = pVert[23].tv = pVert[35].tv = pVert[47].tv = texRect.bottom;
    pVert[8].pos.x = pVert[10].pos.x = fRightMiddle; // left top X
    pVert[20].pos.x = pVert[22].pos.x = fRightMiddle + m_fXShadow;
    pVert[32].pos.x = pVert[34].pos.x = fRightMiddle + m_fXDeltaPress;
    pVert[44].pos.x = pVert[46].pos.x = fRightMiddle + m_fXDeltaPress + m_fXShadowPress;
    pVert[8].pos.y = pVert[9].pos.y = static_cast<float>(m_rect.top); // left top Y
    pVert[20].pos.y = pVert[21].pos.y = m_rect.top + m_fYShadow;
    pVert[32].pos.y = pVert[33].pos.y = m_rect.top + m_fYDeltaPress;
    pVert[44].pos.y = pVert[45].pos.y = m_rect.top + m_fYDeltaPress + m_fYShadowPress;
    pVert[9].pos.x = pVert[11].pos.x = static_cast<float>(m_rect.right); // right top X
    pVert[21].pos.x = pVert[23].pos.x = m_rect.right + m_fXShadow;
    pVert[33].pos.x = pVert[35].pos.x = m_rect.right + m_fXDeltaPress;
    pVert[45].pos.x = pVert[47].pos.x = m_rect.right + m_fXDeltaPress + m_fXShadowPress;
    pVert[10].pos.y = pVert[11].pos.y = static_cast<float>(m_rect.bottom); // left bottom Y
    pVert[22].pos.y = pVert[23].pos.y = m_rect.bottom + m_fYShadow;
    pVert[34].pos.y = pVert[35].pos.y = m_rect.bottom + m_fYDeltaPress;
    pVert[46].pos.y = pVert[47].pos.y = m_rect.bottom + m_fYDeltaPress + m_fYShadowPress;
    // fill middle of button
    auto button_middle = Config::GetOrGet<std::string>(configs, "buttonMiddle", {});
    if (!button_middle.empty()) {
        m_idUnSelectMiddle = pPictureService->GetImageNum(m_sGroupName.c_str(), button_middle.c_str());
    }
    auto select_button_middle = Config::GetOrGet<std::string>(configs, "selectButtonMiddle", {});
    if (!select_button_middle.empty()) {
        m_idSelectMiddle = pPictureService->GetImageNum(m_sGroupName.c_str(), select_button_middle.c_str());
    }
    pPictureService->GetTexturePos(m_idUnSelectMiddle, texRect);
    m_bCurrentSelected = false;
    pVert[4].tu = pVert[16].tu = pVert[28].tu = pVert[40].tu = texRect.left;
    pVert[4].tv = pVert[16].tv = pVert[28].tv = pVert[40].tv = texRect.top;
    pVert[5].tu = pVert[17].tu = pVert[29].tu = pVert[41].tu = texRect.right;
    pVert[5].tv = pVert[17].tv = pVert[29].tv = pVert[41].tv = texRect.top;
    pVert[6].tu = pVert[18].tu = pVert[30].tu = pVert[42].tu = texRect.left;
    pVert[6].tv = pVert[18].tv = pVert[30].tv = pVert[42].tv = texRect.bottom;
    pVert[7].tu = pVert[19].tu = pVert[31].tu = pVert[43].tu = texRect.right;
    pVert[7].tv = pVert[19].tv = pVert[31].tv = pVert[43].tv = texRect.bottom;
    pVert[4].pos.x = pVert[6].pos.x = fLeftMiddle; // left
    pVert[16].pos.x = pVert[18].pos.x = fLeftMiddle + m_fXShadow;
    pVert[28].pos.x = pVert[30].pos.x = fLeftMiddle + m_fXDeltaPress;
    pVert[40].pos.x = pVert[42].pos.x = fLeftMiddle + m_fXDeltaPress + m_fXShadowPress;
    pVert[4].pos.y = pVert[5].pos.y = static_cast<float>(m_rect.top); // top
    pVert[16].pos.y = pVert[17].pos.y = m_rect.top + m_fYShadow;
    pVert[28].pos.y = pVert[29].pos.y = m_rect.top + m_fYDeltaPress;
    pVert[40].pos.y = pVert[41].pos.y = m_rect.top + m_fYDeltaPress + m_fYShadowPress;
    pVert[5].pos.x = pVert[7].pos.x = fRightMiddle; // right
    pVert[17].pos.x = pVert[19].pos.x = fRightMiddle + m_fXShadow;
    pVert[29].pos.x = pVert[31].pos.x = fRightMiddle + m_fXDeltaPress;
    pVert[41].pos.x = pVert[43].pos.x = fRightMiddle + m_fXDeltaPress + m_fXShadowPress;
    pVert[6].pos.y = pVert[7].pos.y = static_cast<float>(m_rect.bottom); // bottom
    pVert[18].pos.y = pVert[19].pos.y = m_rect.bottom + m_fYShadow;
    pVert[30].pos.y = pVert[31].pos.y = m_rect.bottom + m_fYDeltaPress;
    pVert[42].pos.y = pVert[43].pos.y = m_rect.bottom + m_fYDeltaPress + m_fYShadowPress;

    m_nIndx /= 3;

    m_rs->UnLockVertexBuffer(m_idVBuf);
    m_rs->UnLockIndexBuffer(m_idIBuf);

    // fill video fragment
    for (i = 0; i < 8; i++) {
        m_v[i].color = i < 4 ? m_dwFaceColor : m_dwPressedFaceColor;
        m_v[i].pos.z = 1.f;
    }
    m_v[0].tu = m_v[4].tu = 0.f;
    m_v[0].tv = m_v[4].tv = 0.f;
    m_v[1].tu = m_v[5].tu = 1.f;
    m_v[1].tv = m_v[5].tv = 0.f;
    m_v[2].tu = m_v[6].tu = 0.f;
    m_v[2].tv = m_v[6].tv = 1.f;
    m_v[3].tu = m_v[7].tu = 1.f;
    m_v[3].tv = m_v[7].tv = 1.f;

    if (m_bVideoToBack) {
        fLeftMiddle = static_cast<float>(m_rect.left);
        fRightMiddle = static_cast<float>(m_rect.right);
    }

    m_v[4].pos.x = (m_v[0].pos.x = fLeftMiddle) + m_fXDeltaPress;
    m_v[4].pos.y = (m_v[0].pos.y = static_cast<float>(m_rect.top)) + m_fYDeltaPress;

    m_v[5].pos.x = (m_v[1].pos.x = fRightMiddle) + m_fXDeltaPress;
    m_v[5].pos.y = (m_v[1].pos.y = static_cast<float>(m_rect.top)) + m_fYDeltaPress;

    m_v[6].pos.x = (m_v[2].pos.x = fLeftMiddle) + m_fXDeltaPress;
    m_v[6].pos.y = (m_v[2].pos.y = static_cast<float>(m_rect.bottom)) + m_fYDeltaPress;

    m_v[7].pos.x = (m_v[3].pos.x = fRightMiddle) + m_fXDeltaPress;
    m_v[7].pos.y = (m_v[3].pos.y = static_cast<float>(m_rect.bottom)) + m_fYDeltaPress;
}

void CXI_TEXTBUTTON::ReleaseAll() {
    PICTURE_TEXTURE_RELEASE(pPictureService, m_sGroupName.c_str(), m_idTex);
    TEXTURE_RELEASE(m_rs, m_idShadowTex);
    STORM_DELETE(m_sString);
    VERTEX_BUFFER_RELEASE(m_rs, m_idVBuf);
    INDEX_BUFFER_RELEASE(m_rs, m_idIBuf);
    VIDEOTEXTURE_RELEASE(m_rs, m_pTex);
    FONT_RELEASE(m_rs, m_nFontNum);
}

int CXI_TEXTBUTTON::CommandExecute(int wActCode)
{
    if (m_bUse)
    {
        switch (wActCode)
        {
        case ACTION_MOUSECLICK:
        case ACTION_ACTIVATE:
        case ACTION_MOUSEDBLCLICK:
            if (m_bMakeActionInDeclick)
                m_nPressedDelay = 0;
            else
                m_nPressedDelay = m_nMaxDelay;
            break;
        }
    }
    return -1;
}

bool CXI_TEXTBUTTON::IsClick(int buttonID, int32_t xPos, int32_t yPos)
{
    if (xPos >= m_rect.left && xPos <= m_rect.right && yPos >= m_rect.top && yPos <= m_rect.bottom && m_bClickable &&
        m_bUse)
        return true;

    return false;
}

void CXI_TEXTBUTTON::ChangePosition(XYRECT &rNewPos)
{
    m_rect = rNewPos;
    FillPositionIntoVertices();
}

void CXI_TEXTBUTTON::SaveParametersToIni()
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

uint32_t CXI_TEXTBUTTON::MessageProc(int32_t msgcode, MESSAGE &message)
{
    switch (msgcode)
    {
    case 0: // change the text on the button
    {
        const std::string &param = message.String();
        STORM_DELETE(m_sString);
        m_idString = -1;
        if (param[0] == '#')
        {
            {
                const auto len = param.size();
                if ((m_sString = new char[len]) == nullptr)
                {
                    throw std::runtime_error("allocate memory error");
                }
                memcpy(m_sString, param.c_str() + 1, len);
            }
        }
        else if (core.GetTargetEngineVersion() <= storm::ENGINE_VERSION::PIRATES_OF_THE_CARIBBEAN)
        {
            const auto len = param.size();
            if ((m_sString = new char[len + 1]) == nullptr)
            {
                throw std::runtime_error("allocate memory error");
            }
            memcpy(m_sString, param.c_str(), len + 1);
        }
        else
        {
            m_idString = pStringService->GetStringNum(param.c_str());
        }
    }
    break;
    case 1: // change button position
        XYRECT newPos;
        newPos.left = message.Long();
        newPos.top = message.Long();
        newPos.right = message.Long();
        newPos.bottom = message.Long();
        ChangePosition(newPos);
        break;
    }
    return 0;
}

void CXI_TEXTBUTTON::SetUsing(bool bUsing)
{
    m_bUse = bUsing;
    m_nPressedDelay = 0;
}

void CXI_TEXTBUTTON::MakeLClickPreaction()
{
    const auto mouse_pos = ptrOwner->GetMousePoint();
    if (mouse_pos.x >= m_rect.left && mouse_pos.x <= m_rect.right && mouse_pos.y >= m_rect.top &&
        mouse_pos.y <= m_rect.bottom)
    {
        m_nPressedDelay = m_nMaxDelay;
    }
}

void CXI_TEXTBUTTON::FillPositionIntoVertices()
{
    int32_t i;
    FXYRECT texRect;
    XYRECT natureRect;

    auto *pVert = static_cast<XI_ONETEX_VERTEX *>(m_rs->LockVertexBuffer(m_idVBuf));

    if (m_idShadowTex >= 0)
    {
        i = m_nVert - 8;
        const auto fhorzoffset = (m_rect.right - m_rect.left) * (m_fShadowScale - 1.f) * .5f;
        const auto fvertoffset = (m_rect.bottom - m_rect.top) * (m_fShadowScale - 1.f) * .5f;

        pVert[i + 0].pos.x = m_rect.left - fhorzoffset + m_fXShadow;
        pVert[i + 0].pos.y = m_rect.top - fvertoffset + m_fYShadow;
        pVert[i + 1].pos.x = m_rect.left - fhorzoffset + m_fXShadow;
        pVert[i + 1].pos.y = m_rect.bottom + fvertoffset + m_fYShadow;
        pVert[i + 2].pos.x = m_rect.right + fhorzoffset + m_fXShadow;
        pVert[i + 2].pos.y = m_rect.top - fvertoffset + m_fYShadow;
        pVert[i + 3].pos.x = m_rect.right + fhorzoffset + m_fXShadow;
        pVert[i + 3].pos.y = m_rect.bottom + fvertoffset + m_fYShadow;

        pVert[i + 4].pos.x = m_rect.left - fhorzoffset + m_fXDeltaPress + m_fXShadow;
        pVert[i + 4].pos.y = m_rect.top - fvertoffset + m_fYDeltaPress + m_fYShadow;
        pVert[i + 5].pos.x = m_rect.left - fhorzoffset + m_fXDeltaPress + m_fXShadow;
        pVert[i + 5].pos.y = m_rect.bottom + fvertoffset + m_fYDeltaPress + m_fYShadow;
        pVert[i + 6].pos.x = m_rect.right + fhorzoffset + m_fXDeltaPress + m_fXShadow;
        pVert[i + 6].pos.y = m_rect.top - fvertoffset + m_fYDeltaPress + m_fYShadow;
        pVert[i + 7].pos.x = m_rect.right + fhorzoffset + m_fXDeltaPress + m_fXShadow;
        pVert[i + 7].pos.y = m_rect.bottom + fvertoffset + m_fYDeltaPress + m_fYShadow;
    }

    // fill left side of button
    pPictureService->GetTexturePos(m_idUnSelectLeft, texRect);
    pPictureService->GetTexturePos(m_idUnSelectLeft, natureRect);
    float fLeftMiddle = static_cast<float>(m_rect.left + natureRect.right - natureRect.left);
    pVert[0].pos.x = pVert[2].pos.x = static_cast<float>(m_rect.left);
    pVert[12].pos.x = pVert[14].pos.x = m_rect.left + m_fXShadow;
    pVert[24].pos.x = pVert[26].pos.x = m_rect.left + m_fXDeltaPress;
    pVert[36].pos.x = pVert[38].pos.x = m_rect.left + m_fXDeltaPress + m_fXShadowPress;
    pVert[0].pos.y = pVert[1].pos.y = static_cast<float>(m_rect.top);
    pVert[12].pos.y = pVert[13].pos.y = m_rect.top + m_fYShadow;
    pVert[24].pos.y = pVert[25].pos.y = m_rect.top + m_fYDeltaPress;
    pVert[36].pos.y = pVert[37].pos.y = m_rect.top + m_fYDeltaPress + m_fYShadowPress;
    pVert[1].pos.x = pVert[3].pos.x = fLeftMiddle;
    pVert[13].pos.x = pVert[15].pos.x = fLeftMiddle + m_fXShadow;
    pVert[25].pos.x = pVert[27].pos.x = fLeftMiddle + m_fXDeltaPress;
    pVert[37].pos.x = pVert[39].pos.x = fLeftMiddle + m_fXDeltaPress + m_fXShadowPress;
    pVert[2].pos.y = pVert[3].pos.y = static_cast<float>(m_rect.bottom);
    pVert[14].pos.y = pVert[15].pos.y = m_rect.bottom + m_fYShadow;
    pVert[26].pos.y = pVert[27].pos.y = m_rect.bottom + m_fYDeltaPress;
    pVert[38].pos.y = pVert[39].pos.y = m_rect.bottom + m_fYDeltaPress + m_fYShadowPress;

    // fill right side of button
    if (m_idUnSelectRight != -1)
    {
        pPictureService->GetTexturePos(m_idUnSelectRight, texRect);
        pPictureService->GetTexturePos(m_idUnSelectRight, natureRect);
    }
    else
    {
        pPictureService->GetTexturePos(TEXTURE_MODIFY_HORZFLIP, m_idUnSelectLeft, texRect);
        pPictureService->GetTexturePos(m_idUnSelectLeft, natureRect);
    }
    float fRightMiddle = m_rect.right - static_cast<float>(natureRect.right - natureRect.left);
    pVert[8].pos.x = pVert[10].pos.x = fRightMiddle; // left top X
    pVert[20].pos.x = pVert[22].pos.x = fRightMiddle + m_fXShadow;
    pVert[32].pos.x = pVert[34].pos.x = fRightMiddle + m_fXDeltaPress;
    pVert[44].pos.x = pVert[46].pos.x = fRightMiddle + m_fXDeltaPress + m_fXShadowPress;
    pVert[8].pos.y = pVert[9].pos.y = static_cast<float>(m_rect.top); // left top Y
    pVert[20].pos.y = pVert[21].pos.y = m_rect.top + m_fYShadow;
    pVert[32].pos.y = pVert[33].pos.y = m_rect.top + m_fYDeltaPress;
    pVert[44].pos.y = pVert[45].pos.y = m_rect.top + m_fYDeltaPress + m_fYShadowPress;
    pVert[9].pos.x = pVert[11].pos.x = static_cast<float>(m_rect.right); // right top X
    pVert[21].pos.x = pVert[23].pos.x = m_rect.right + m_fXShadow;
    pVert[33].pos.x = pVert[35].pos.x = m_rect.right + m_fXDeltaPress;
    pVert[45].pos.x = pVert[47].pos.x = m_rect.right + m_fXDeltaPress + m_fXShadowPress;
    pVert[10].pos.y = pVert[11].pos.y = static_cast<float>(m_rect.bottom); // left bottom Y
    pVert[22].pos.y = pVert[23].pos.y = m_rect.bottom + m_fYShadow;
    pVert[34].pos.y = pVert[35].pos.y = m_rect.bottom + m_fYDeltaPress;
    pVert[46].pos.y = pVert[47].pos.y = m_rect.bottom + m_fYDeltaPress + m_fYShadowPress;

    // fill middle of button
    pPictureService->GetTexturePos(m_idUnSelectMiddle, texRect);
    pVert[4].pos.x = pVert[6].pos.x = fLeftMiddle; // left
    pVert[16].pos.x = pVert[18].pos.x = fLeftMiddle + m_fXShadow;
    pVert[28].pos.x = pVert[30].pos.x = fLeftMiddle + m_fXDeltaPress;
    pVert[40].pos.x = pVert[42].pos.x = fLeftMiddle + m_fXDeltaPress + m_fXShadowPress;
    pVert[4].pos.y = pVert[5].pos.y = static_cast<float>(m_rect.top); // top
    pVert[16].pos.y = pVert[17].pos.y = m_rect.top + m_fYShadow;
    pVert[28].pos.y = pVert[29].pos.y = m_rect.top + m_fYDeltaPress;
    pVert[40].pos.y = pVert[41].pos.y = m_rect.top + m_fYDeltaPress + m_fYShadowPress;
    pVert[5].pos.x = pVert[7].pos.x = fRightMiddle; // right
    pVert[17].pos.x = pVert[19].pos.x = fRightMiddle + m_fXShadow;
    pVert[29].pos.x = pVert[31].pos.x = fRightMiddle + m_fXDeltaPress;
    pVert[41].pos.x = pVert[43].pos.x = fRightMiddle + m_fXDeltaPress + m_fXShadowPress;
    pVert[6].pos.y = pVert[7].pos.y = static_cast<float>(m_rect.bottom); // bottom
    pVert[18].pos.y = pVert[19].pos.y = m_rect.bottom + m_fYShadow;
    pVert[30].pos.y = pVert[31].pos.y = m_rect.bottom + m_fYDeltaPress;
    pVert[42].pos.y = pVert[43].pos.y = m_rect.bottom + m_fYDeltaPress + m_fYShadowPress;

    m_rs->UnLockVertexBuffer(m_idVBuf);

    if (m_bVideoToBack)
    {
        fLeftMiddle = static_cast<float>(m_rect.left);
        fRightMiddle = static_cast<float>(m_rect.right);
    }

    m_v[4].pos.x = (m_v[0].pos.x = fLeftMiddle) + m_fXDeltaPress;
    m_v[4].pos.y = (m_v[0].pos.y = static_cast<float>(m_rect.top)) + m_fYDeltaPress;

    m_v[5].pos.x = (m_v[1].pos.x = fRightMiddle) + m_fXDeltaPress;
    m_v[5].pos.y = (m_v[1].pos.y = static_cast<float>(m_rect.top)) + m_fYDeltaPress;

    m_v[6].pos.x = (m_v[2].pos.x = fLeftMiddle) + m_fXDeltaPress;
    m_v[6].pos.y = (m_v[2].pos.y = static_cast<float>(m_rect.bottom)) + m_fYDeltaPress;

    m_v[7].pos.x = (m_v[3].pos.x = fRightMiddle) + m_fXDeltaPress;
    m_v[7].pos.y = (m_v[3].pos.y = static_cast<float>(m_rect.bottom)) + m_fYDeltaPress;
}
