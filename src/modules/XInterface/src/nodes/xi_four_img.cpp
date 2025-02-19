#include "xi_four_img.h"

#include <cstdio>

#include "core.h"

using namespace Storm::Filesystem;
using namespace Storm::Math;

CXI_FOURIMAGE::CXI_FOURIMAGE()
{
    m_rs = nullptr;
    m_bClickable = true;
    m_oneStrFont = -1;
    m_twoStrFont = -1;
    m_nNodeType = NODETYPE_FOURIMAGE;
    m_oneBadTexture = m_twoBadTexture = -1;
    vBuf = -1;
    m_nTexturesQuantity = 0;
    m_nTextureId = nullptr;
}

CXI_FOURIMAGE::~CXI_FOURIMAGE()
{
    ReleaseAll();
}

int CXI_FOURIMAGE::CommandExecute(int wActCode)
{
    auto retVal = -1;
    if (m_bUse)
    {
        auto newSelectItem = m_nSelectItem;

        switch (wActCode)
        {
        case ACTION_RIGHTSTEP:
            newSelectItem++;
            break;
        case ACTION_LEFTSTEP:
            newSelectItem--;
            break;
        case ACTION_UPSTEP: {
            auto *pvdat = core.Event("FI_UpCom", "l", m_nSelectItem);
            if (pvdat == nullptr || pvdat->GetInt() == 0)
                newSelectItem -= 2;
        }
        break;
        case ACTION_DOWNSTEP:
            newSelectItem += 2;
            break;
        case ACTION_ACTIVATE:
            break;
        case ACTION_SPEEDRIGHT:
            newSelectItem++;
            break;
        case ACTION_SPEEDLEFT:
            newSelectItem--;
            break;
        case ACTION_DEACTIVATE:
            break;
        case ACTION_MOUSECLICK:
            int i;
            for (i = 0; i < 4; i++)
                if (m_MousePoint.x >= m_imgRect[i].left && m_MousePoint.x <= m_imgRect[i].right &&
                    m_MousePoint.y >= m_imgRect[i].top && m_MousePoint.y <= m_imgRect[i].bottom)
                    break;
            if (i < 4)
            {
                newSelectItem = i;
                if (m_nSelectItem == newSelectItem && IsCurrentNode())
                {
                    /*core.Event("ievnt_command","ss","activate",m_nodeName);
                    for(int n=0; n<COMMAND_QUANTITY; n++)
                      if(pCommandsList[n].code==wActCode) break;
                    if(n<COMMAND_QUANTITY) m_nCurrentCommandNumber = n;*/
                    retVal = ACTION_ACTIVATE;
                }
            }
            break;
        }

        while (newSelectItem < 0)
            newSelectItem += 4;
        while (newSelectItem > 3)
            newSelectItem -= 4;

        if (m_bUsed[newSelectItem])
            m_nSelectItem = newSelectItem;

        // set new current item
        auto *tmpAttr = core.Entity_GetAttributeClass(g_idInterface, "FourImage");
        tmpAttr->SetAttributeUseDword("current", m_nSelectItem);
    }
    return retVal;
}

void CXI_FOURIMAGE::Draw(bool bSelected, uint32_t Delta_Time)
{
    // GUARD(void CXI_FOURIMAGE::Draw())

    if (m_bUse)
    {
        Update(bSelected, Delta_Time);

        // draw one picture
        for (auto i = 0; i < 4; i++)
        {
            if (m_oneTexID[i] == -1 || m_oneTexID[i] >= m_nTexturesQuantity)
                continue;
            if (m_oneImgID[i] == -1)
                m_rs->TextureSet(0, m_oneBadTexture);
            else
                m_rs->TextureSet(0, m_nTextureId[m_oneTexID[i]]);
            m_rs->DrawPrimitive(D3DPT_TRIANGLESTRIP, vBuf, sizeof(XI_ONETEX_VERTEX), i * 4, 2, "iFourImages_main");
        }

        // create select border
        if (m_bShowBorder && bSelected)
        {
            XI_ONLYONETEX_VERTEX pV[4];
            FXYRECT textureRect;
            for (auto i = 0; i < 4; i++)
                pV[i].pos.z = 1.f;
            pPictureService->GetTexturePos(m_nBorderPicture, textureRect);
            pV[0].tu = textureRect.left;
            pV[0].tv = textureRect.top;
            pV[1].tu = textureRect.left;
            pV[1].tv = textureRect.bottom;
            pV[2].tu = textureRect.right;
            pV[2].tv = textureRect.top;
            pV[3].tu = textureRect.right;
            pV[3].tv = textureRect.bottom;
            pV[0].pos.x = static_cast<float>(m_imgRect[m_nSelectItem].left);
            pV[0].pos.y = static_cast<float>(m_imgRect[m_nSelectItem].top);
            pV[1].pos.x = static_cast<float>(m_imgRect[m_nSelectItem].left);
            pV[1].pos.y = static_cast<float>(m_imgRect[m_nSelectItem].bottom);
            pV[2].pos.x = static_cast<float>(m_imgRect[m_nSelectItem].right);
            pV[2].pos.y = static_cast<float>(m_imgRect[m_nSelectItem].top);
            pV[3].pos.x = static_cast<float>(m_imgRect[m_nSelectItem].right);
            pV[3].pos.y = static_cast<float>(m_imgRect[m_nSelectItem].bottom);
            // show select border
            m_rs->TextureSet(0, m_texBorder);
            m_rs->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, XI_ONLYONETEX_FVF, 2, pV, sizeof(XI_ONLYONETEX_VERTEX),
                                  "iFourImages_border");
        }

        // draw two picture
        for (auto i = 0; i < 4; i++)
        {
            if (m_twoTexID[i] == -1 || m_twoTexID[i] >= m_nTexturesQuantity)
                continue;
            if (m_twoImgID[i] == -1)
                m_rs->TextureSet(0, m_twoBadTexture);
            else
                m_rs->TextureSet(0, m_nTextureId[m_twoTexID[i]]);
            m_rs->DrawPrimitive(D3DPT_TRIANGLESTRIP, vBuf, sizeof(XI_ONETEX_VERTEX), 16 + i * 4, 2, "iFourImages_main");
        }

        // out to screen the strings if that needed
        if (bUseOneString || bUseTwoString)
        {
            auto nAlignment1 = PR_ALIGN_CENTER, nAlignment2 = PR_ALIGN_CENTER;
            if (m_xOneOffset > 0)
                nAlignment1 = PR_ALIGN_RIGHT;
            else if (m_xOneOffset < 0)
                nAlignment1 = PR_ALIGN_LEFT;
            if (m_xTwoOffset > 0)
                nAlignment2 = PR_ALIGN_RIGHT;
            else if (m_xTwoOffset < 0)
                nAlignment2 = PR_ALIGN_LEFT;

            for (auto i = 0; i < 4; i++)
            {
                int32_t posX, posY;
                posX = (m_imgRect[i].left + m_imgRect[i].right) / 2;
                const auto color = 0xFFFFFFFF;
                /*if(i==m_nSelectItem) color=m_dwCurSelectColor;
                else color=m_dwBaseColor;*/

                if (bUseOneString)
                {
                    posY = m_imgRect[i].top + m_nOneStrOffset;

                    if (m_pOneStr[i] != nullptr)
                        m_rs->ExtPrint(m_oneStrFont, color, 0, nAlignment1, false, 1.f, m_screenSize.x, m_screenSize.y,
                                       posX + m_xOneOffset, posY, "%s", m_pOneStr[i]);
                    else if (m_oneStr[i] != -1)
                        m_rs->ExtPrint(m_oneStrFont, color, 0, nAlignment1, false, 1.f, m_screenSize.x, m_screenSize.y,
                                       posX + m_xOneOffset, posY, "%s", pStringService->GetString(m_oneStr[i]));
                }
                if (bUseTwoString)
                {
                    posY = m_imgRect[i].top + m_nTwoStrOffset;
                    if (m_pTwoStr[i] != nullptr)
                        m_rs->ExtPrint(m_twoStrFont, color, 0, nAlignment2, false, 1.f, m_screenSize.x, m_screenSize.y,
                                       posX + m_xTwoOffset, posY, "%s", m_pTwoStr[i]);
                    else if (m_twoStr[i] != -1)
                        m_rs->ExtPrint(m_twoStrFont, color, 0, nAlignment2, false, 1.f, m_screenSize.x, m_screenSize.y,
                                       posX + m_xTwoOffset, posY, "%s", pStringService->GetString(m_twoStr[i]));
                }
            }
        }
    }

    // UNGUARD
}

bool CXI_FOURIMAGE::Init(const Config& node_config, const Config& def_config, VDX9RENDER *rs,
                         XYRECT &hostRect, XYPOINT &ScreenSize) {
    if (!CINODE::Init(node_config, def_config, rs, hostRect, ScreenSize)) {
        return false;
    }
    // screen position for that is host screen position
    memcpy(&m_rect, &m_hostRect, sizeof(m_hostRect));
    FillVertex();
    return true;
}

void CXI_FOURIMAGE::LoadIni(const Config& node_config, const Config& def_config) {
    std::pair<const Config&, const Config&> configs{node_config, def_config};
    // get base color
    auto common_color = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "commonColor", {255, 128, 128, 128});
    m_dwBaseColor = ARGB(common_color.x, common_color.y, common_color.z, common_color.w);

    // get color for light select image
    auto light_select_color = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "lightSelectCol", {255});
    m_dwLightSelectColor = ARGB(light_select_color.x, light_select_color.y, light_select_color.z, light_select_color.w);
    m_dwCurSelectColor = m_dwLightSelectColor;

    // get color for dark select image
    auto dark_select_color = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "darkSelectCol", {255, 150, 150, 150});
    m_dwDarkSelectColor = ARGB(dark_select_color.x, dark_select_color.y, dark_select_color.z, dark_select_color.w);

    // get blind parameters
    m_bColorType = true; // select item used select color
    m_nBlindCounter = 0; // currend blind counter
    m_nMaxBlindCounter = Config::GetOrGet<std::int64_t>(configs, "wBlindDelay", -1);
    m_bDoBlind = m_nMaxBlindCounter > 0;

    // get one string parameters
    m_foreColOneStr = ARGB(255, 255, 255, 255);
    m_backColOneStr = ARGB(0, 0, 0, 0);
    m_nOneStrOffset = 0;
    m_xOneOffset = 0;
    m_oneStrFont = -1;
    const auto one_string = Config::GetOrGet<std::vector<std::vector<std::string>>>(configs, "oneString", {});
    if ((bUseOneString = !one_string.empty())) {
        std::stringstream ss;
        for (const auto& line : one_string) {
            for (const auto& each : line) {
                ss << each << ',';
            }
        }
        std::string one_string_str{ss.str()};
        std::size_t res{0};

        std::size_t idx_begin = one_string_str.find("font:");
        std::size_t idx_end = one_string_str.find('}', idx_begin);
        if (idx_begin != std::string::npos && idx_end != std::string::npos) {
            idx_begin += 5;
            std::string font{};
            res = sscanf_s(one_string_str.substr(idx_begin, idx_end - idx_begin).c_str(), "%[^0]s", font.data(), 20);
            if (res == 1) {
                m_oneStrFont = m_rs->LoadFont(font);
            }
        }
        if (m_oneStrFont == -1) {
            core.Trace("can not load font:");
        }

        idx_begin = one_string_str.find("off:(");
        idx_end = one_string_str.find(')', idx_begin);
        if (idx_begin != std::string::npos && idx_end != std::string::npos) {
            idx_begin += 5;
            int x{0}, y{0};
            res = sscanf_s(one_string_str.substr(idx_begin, idx_end - idx_begin).c_str(), "%d,%d", &x, &y);
            if (res == 2) {
                m_xOneOffset = x;
                m_nOneStrOffset = y;
            }
        }

        idx_begin = one_string_str.find("fc:{");
        idx_end = one_string_str.find('}', idx_begin);
        if (idx_begin != std::string::npos && idx_end != std::string::npos) {
            idx_begin += 4;
            int a{0}, r{0}, g{0}, b{0};
            res = sscanf_s(one_string_str.substr(idx_begin, idx_end - idx_begin).c_str(), "%d,%d,%d,%d", &a, &r, &g, &b);
            if (res == 4) {
                m_foreColOneStr = ARGB(a, r, g, b);
            }
        }

        idx_begin = one_string_str.find("bc:{");
        idx_end = one_string_str.find('}', idx_begin);
        if (idx_begin != std::string::npos && idx_end != std::string::npos) {
            idx_begin += 4;
            int a{0}, r{0}, g{0}, b{0};
            res = sscanf_s(one_string_str.substr(idx_begin, idx_end - idx_begin).c_str(), "%d,%d,%d,%d", &a, &r, &g, &b);
            if (res == 4) {
                m_backColOneStr = ARGB(a, r, g, b);
            }
        }
    }

    // get two string parameters
    m_foreColTwoStr = ARGB(255, 255, 255, 255);
    m_backColTwoStr = ARGB(0, 0, 0, 0);
    m_nTwoStrOffset = 0;
    m_xTwoOffset = 0;
    m_twoStrFont = -1;
    const auto two_string = Config::GetOrGet<std::vector<std::vector<std::string>>>(configs, "twoString", {});
    if ((bUseTwoString = !two_string.empty())) {
        std::stringstream ss;
        for (const auto& line : two_string) {
            for (const auto& each : line) {
                ss << each << ',';
            }
        }
        std::string two_string_str{ss.str()};
        std::size_t res{0};

        std::size_t idx_begin = two_string_str.find("font:");
        std::size_t idx_end = two_string_str.find('}', idx_begin);

        if (idx_begin != std::string::npos && idx_end != std::string::npos) {
            idx_begin += 5;
            std::string font{};
            res = sscanf_s(two_string_str.substr(idx_begin, idx_end - idx_begin).c_str(), "%[^0]s", font.data(), 20);
            if (res == 1) {
                m_twoStrFont = m_rs->LoadFont(font);
            }
        }
        if (m_twoStrFont == -1) {
            core.Trace("can not load font:");
        }

        idx_begin = two_string_str.find("off:(");
        idx_end = two_string_str.find(')', idx_begin);
        if (idx_begin != std::string::npos && idx_end != std::string::npos) {
            idx_begin += 5;
            int x{0}, y{0};
            res = sscanf_s(two_string_str.substr(idx_begin, idx_end - idx_begin).c_str(), "%d,%d", &x, &y);
            if (res == 2) {
                m_xTwoOffset = x;
                m_nTwoStrOffset = y;
            }
        }

        idx_begin = two_string_str.find("fc:{");
        idx_end = two_string_str.find('}', idx_begin);
        if (idx_begin != std::string::npos && idx_end != std::string::npos) {
            idx_begin += 4;
            int a{0}, r{0}, g{0}, b{0};
            res = sscanf_s(two_string_str.substr(idx_begin, idx_end - idx_begin).c_str(), "%d,%d,%d,%d", &a, &r, &g, &b);
            if (res == 4) {
                m_foreColTwoStr = ARGB(a, r, g, b);
            }
        }

        idx_begin = two_string_str.find("bc:{");
        idx_end = two_string_str.find('}', idx_begin);
        if (idx_begin != std::string::npos && idx_end != std::string::npos) {
            idx_begin += 4;
            int a{0}, r{0}, g{0}, b{0};
            res = sscanf_s(two_string_str.substr(idx_begin, idx_end - idx_begin).c_str(), "%d,%d,%d,%d", &a, &r, &g, &b);
            if (res == 4) {
                m_backColTwoStr = ARGB(a, r, g, b);
            }
        }
    }

    auto *pAttribute = core.Entity_GetAttributeClass(g_idInterface, "FourImage");
    if (pAttribute != nullptr) {
        m_nSelectItem = pAttribute->GetAttributeAsDword("current", 0);
        // get bad picture
        const char *sBadPict = pAttribute->GetAttribute("BadOnePicture");
        m_oneBadTexture = sBadPict == nullptr
            ? -1
            : m_rs->TextureCreate(sBadPict);

        sBadPict = pAttribute->GetAttribute("BadTwoPicture");
        m_twoBadTexture = sBadPict == nullptr
            ? -1
            : m_rs->TextureCreate(sBadPict);

        // get textures
        auto *pA = pAttribute->GetAttributeClass("ImagesGroup");
        if (pA == nullptr) {
            m_nTexturesQuantity = 0;
        } else {
            m_nTexturesQuantity = pA->GetAttributesNum();
            m_nTextureId = new int32_t[m_nTexturesQuantity];
            m_sGroupName = new char *[m_nTexturesQuantity];
            if (m_sGroupName == nullptr || m_nTextureId == nullptr) {
                throw std::runtime_error("Allocate memory error");
            }
            for (int i = 0; i < m_nTexturesQuantity; i++) {
                const char *stmp = pA->GetAttribute(i);
                if (stmp == nullptr) {
                    m_sGroupName[i] = nullptr;
                } else {
                    const auto len = strlen(stmp) + 1;
                    if ((m_sGroupName[i] = new char[len]) == nullptr) {
                        throw std::runtime_error("Allocate memory error");
                    }
                    memcpy(m_sGroupName[i], stmp, len);
                }
            }
        }
        for (auto i = 0; i < m_nTexturesQuantity; i++) {
            m_nTextureId[i] = pPictureService->GetTextureID(m_sGroupName[i]);
        }

        for (auto i = 0; i < 4; i++) {
            auto *pAttrTmp = pAttribute->GetAttributeClass("pic" + std::to_string(i + 1));
            if (pAttrTmp != nullptr) {
                m_bUsed[i] = pAttrTmp->GetAttributeAsDword("selected", 0) != 0;
                m_oneTexID[i] = pAttrTmp->GetAttributeAsDword("tex1", -1);
                m_twoTexID[i] = pAttrTmp->GetAttributeAsDword("tex2", -1);
                m_oneImgID[i] = m_oneTexID[i] != -1 && m_oneTexID[i] < m_nTexturesQuantity
                    ? m_oneImgID[i] = pPictureService->GetImageNum(m_sGroupName[m_oneTexID[i]], pAttrTmp->GetAttribute("img1"))
                    : -1;
                m_twoTexID[i] = m_twoTexID[i] != -1 && m_twoTexID[i] < m_nTexturesQuantity
                    ? m_twoImgID[i] = pPictureService->GetImageNum(m_sGroupName[m_twoTexID[i]], pAttrTmp->GetAttribute("img2"))
                    : -1;
                const char *tmps = pAttrTmp->GetAttribute("str1");
                if (tmps != nullptr && *tmps == '#') {
                    const auto len = strlen(tmps);
                    if ((m_pOneStr[i] = new char[len]) == nullptr) {
                        throw std::runtime_error("allocate memory error");
                    }
                    memcpy(m_pOneStr[i], &tmps[1], len);
                    m_oneStr[i] = -1L;
                } else {
                    m_pOneStr[i] = nullptr;
                    m_oneStr[i] = pStringService->GetStringNum(tmps);
                }
                tmps = pAttrTmp->GetAttribute("str2");
                if (tmps != nullptr && *tmps == '#') {
                    const auto len = strlen(tmps);
                    if ((m_pTwoStr[i] = new char[len]) == nullptr) {
                        throw std::runtime_error("allocate memory error");
                    }
                    memcpy(m_pTwoStr[i], &tmps[1], len);
                    m_twoStr[i] = -1L;
                } else {
                    m_pTwoStr[i] = nullptr;
                    m_twoStr[i] = pStringService->GetStringNum(tmps);
                }
            } else {
                m_oneImgID[i] = -1L;
                m_twoImgID[i] = -1L;
                m_oneStr[i] = -1L;
                m_twoStr[i] = -1L;
                m_pOneStr[i] = nullptr;
                m_pTwoStr[i] = nullptr;
                m_oneTexID[i] = m_oneBadTexture == -1
                    ? -1
                    : 0;
                m_twoTexID[i] = m_twoBadTexture == -1
                    ? -1
                    : 0;
            }
        }
    }

    auto bRelativeRect = Config::GetOrGet<std::int64_t>(configs, "bAbsoluteRectangle", 0) == false;
    for (auto i = 0; i < 4; i++) {
        m_imgRect[i] = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "position" + std::to_string(i + 1), {});
        if (bRelativeRect) {
            GetRelativeRect(m_imgRect[i]);
        }
    }

    // get border picture
    const auto border = Config::GetOrGet<std::vector<std::string>>(configs, "border", {});
    m_bShowBorder = false;
    m_texBorder = -1;
    if (std::size(border) > 1) {
        m_sBorderGroupName = border[0];
        m_texBorder = pPictureService->GetTextureID(m_sBorderGroupName.c_str());
        m_nBorderPicture = pPictureService->GetImageNum(m_sBorderGroupName.c_str(), border[0].c_str());
        m_bShowBorder = m_texBorder != -1 && m_nBorderPicture != -1;
    }

    // create index & vertex buffers
    vBuf = m_rs->CreateVertexBuffer(XI_ONETEX_FVF, 4 * 4 * 2 * sizeof(XI_ONETEX_VERTEX), D3DUSAGE_WRITEONLY);
}

void CXI_FOURIMAGE::ReleaseAll()
{
    VERTEX_BUFFER_RELEASE(m_rs, vBuf);

    TEXTURE_RELEASE(m_rs, m_oneBadTexture);
    TEXTURE_RELEASE(m_rs, m_twoBadTexture);
    // Release all used textures
    for (int i = 0; i < m_nTexturesQuantity; i++)
    {
        PICTURE_TEXTURE_RELEASE(pPictureService, m_sGroupName[i], m_nTextureId[i]);
        STORM_DELETE(m_sGroupName[i]);
    }
    STORM_DELETE(m_sGroupName);
    STORM_DELETE(m_nTextureId);
    PICTURE_TEXTURE_RELEASE(pPictureService, m_sBorderGroupName.c_str(), m_texBorder);

    for (int i = 0; i < 4; i++) {
        STORM_DELETE(m_pOneStr[i]);
        STORM_DELETE(m_pTwoStr[i]);
    }

    // release all image list names
    m_sBorderGroupName.clear();

    FONT_RELEASE(m_rs, m_oneStrFont);
    FONT_RELEASE(m_rs, m_twoStrFont);

    m_nTexturesQuantity = 0;
}

void CXI_FOURIMAGE::FillVertex()
{
    auto *pVert = static_cast<XI_ONETEX_VERTEX *>(m_rs->LockVertexBuffer(vBuf));
    if (pVert != nullptr)
    {
        FXYRECT textRect1, textRect2;

        for (auto i = 0; i < 4; i++)
        {
            auto idx = i * 4;
            // get texture rectangles
            if (m_oneImgID[i] != -1)
                pPictureService->GetTexturePos(m_oneImgID[i], textRect1);
            else
            {
                textRect1.left = 0.f;
                textRect1.top = 0.f;
                textRect1.right = 1.f;
                textRect1.bottom = 1.f;
            }
            if (m_twoImgID[i] != -1)
                pPictureService->GetTexturePos(m_twoImgID[i], textRect2);
            else
            {
                textRect2.left = 0.f;
                textRect2.top = 0.f;
                textRect2.right = 1.f;
                textRect2.bottom = 1.f;
            }

            // left top vertex
            pVert[idx].pos.x = pVert[16 + idx].pos.x = static_cast<float>(m_imgRect[i].left);
            pVert[idx].pos.y = pVert[16 + idx].pos.y = static_cast<float>(m_imgRect[i].top);
            pVert[idx].tu = textRect1.left;
            pVert[idx].tv = textRect1.top;
            pVert[16 + idx].tu = textRect2.left;
            pVert[16 + idx].tv = textRect2.top;
            // left bottom vertex
            pVert[idx + 1].pos.x = pVert[17 + idx].pos.x = static_cast<float>(m_imgRect[i].left);
            pVert[idx + 1].pos.y = pVert[17 + idx].pos.y = static_cast<float>(m_imgRect[i].bottom);
            pVert[idx + 1].tu = textRect1.left;
            pVert[idx + 1].tv = textRect1.bottom;
            pVert[17 + idx].tu = textRect2.left;
            pVert[17 + idx].tv = textRect2.bottom;
            // right top vertex
            pVert[idx + 2].pos.x = pVert[18 + idx].pos.x = static_cast<float>(m_imgRect[i].right);
            pVert[idx + 2].pos.y = pVert[18 + idx].pos.y = static_cast<float>(m_imgRect[i].top);
            pVert[idx + 2].tu = textRect1.right;
            pVert[idx + 2].tv = textRect1.top;
            pVert[18 + idx].tu = textRect2.right;
            pVert[18 + idx].tv = textRect2.top;
            // right bottom vertex
            pVert[idx + 3].pos.x = pVert[19 + idx].pos.x = static_cast<float>(m_imgRect[i].right);
            pVert[idx + 3].pos.y = pVert[19 + idx].pos.y = static_cast<float>(m_imgRect[i].bottom);
            pVert[idx + 3].tu = textRect1.right;
            pVert[idx + 3].tv = textRect1.bottom;
            pVert[19 + idx].tu = textRect2.right;
            pVert[19 + idx].tv = textRect2.bottom;

            for (auto j = 0; j < 4; j++, idx++)
            {
                if (i == m_nSelectItem)
                    pVert[16 + idx].color = m_dwCurSelectColor;
                else
                    pVert[16 + idx].color = m_dwBaseColor;
                pVert[idx].color = m_dwBaseColor;
                pVert[idx].pos.z = pVert[16 + idx].pos.z = 1.f;
            }
        }

        m_rs->UnLockVertexBuffer(vBuf);
    }
}

void CXI_FOURIMAGE::Update(bool bSelected, uint32_t DeltaTime)
{
    //
    auto *pVert = static_cast<XI_ONETEX_VERTEX *>(m_rs->LockVertexBuffer(vBuf));
    if (pVert != nullptr)
    {
        auto idx = 0;
        auto setColor = 0xFFFFFFFF;
        for (auto i = 0; i < 4; i++)
        {
            if (i == m_nSelectItem)
            {
                if (m_bDoBlind && bSelected)
                {
                    if ((m_nBlindCounter -= DeltaTime) <= 0)
                    {
                        m_nBlindCounter = m_nMaxBlindCounter;
                        m_bColorType = !m_bColorType;
                    }

                    const auto ad = ALPHA(m_dwDarkSelectColor);
                    const auto rd = RED(m_dwDarkSelectColor);
                    const auto gd = GREEN(m_dwDarkSelectColor);
                    const auto bd = BLUE(m_dwDarkSelectColor);
                    const auto al = ALPHA(m_dwLightSelectColor);
                    const auto rl = RED(m_dwLightSelectColor);
                    const auto gl = GREEN(m_dwLightSelectColor);
                    const auto bl = BLUE(m_dwLightSelectColor);
                    uint32_t a, r, g, b;
                    if (m_bColorType)
                    {
                        a = (al - ad) * m_nBlindCounter / m_nMaxBlindCounter;
                        r = (rl - rd) * m_nBlindCounter / m_nMaxBlindCounter;
                        g = (gl - gd) * m_nBlindCounter / m_nMaxBlindCounter;
                        b = (bl - bd) * m_nBlindCounter / m_nMaxBlindCounter;
                        m_dwCurSelectColor = ARGB(ad + a, rd + r, gd + g, bd + b);
                    }
                    else
                    {
                        a = (al - ad) * m_nBlindCounter / m_nMaxBlindCounter;
                        r = (rl - rd) * m_nBlindCounter / m_nMaxBlindCounter;
                        g = (gl - gd) * m_nBlindCounter / m_nMaxBlindCounter;
                        b = (bl - bd) * m_nBlindCounter / m_nMaxBlindCounter;
                        m_dwCurSelectColor = ARGB(al - a, rl - r, gl - g, bl - b);
                    }
                }
                else
                    m_dwCurSelectColor = m_dwLightSelectColor;
                setColor = m_dwCurSelectColor;
            }
            else
                setColor = m_dwBaseColor;

            for (auto j = 0; j < 4; j++, idx++)
                if (m_twoTexID[i] != -1)
                    pVert[16 + idx].color = setColor;
                else
                    pVert[idx].color = setColor;
        }

        m_rs->UnLockVertexBuffer(vBuf);
    }
}

bool CXI_FOURIMAGE::IsClick(int buttonID, int32_t xPos, int32_t yPos)
{
    for (auto i = 0; i < 4; i++)
        if (xPos >= m_imgRect[i].left && xPos <= m_imgRect[i].right && yPos >= m_imgRect[i].top &&
            yPos <= m_imgRect[i].bottom && m_bClickable && m_bUse)
        {
            m_MousePoint.x = xPos;
            m_MousePoint.y = yPos;

            return true;
        }

    return false;
}

void CXI_FOURIMAGE::ChangePosition(XYRECT &rNewPos)
{
    const auto nXOffset = rNewPos.left - m_rect.left;
    const auto nYOffset = rNewPos.top - m_rect.top;
    const auto nXToGrow = rNewPos.right - m_rect.right;
    const auto nYToGrow = rNewPos.bottom - m_rect.bottom;

    m_rect = rNewPos;

    for (int32_t n = 0; n < 4; n++)
    {
        m_imgRect[n].left += nXOffset;
        m_imgRect[n].top += nYOffset;
        m_imgRect[n].right += nXOffset;
        m_imgRect[n].bottom += nYOffset;

        // find whether the position is right or bottom
        int32_t nXLessMore = 0;
        int32_t nYLessMore = 0;
        for (int32_t i = 0; i < 4; i++)
        {
            if (i == n)
                continue;
            if (m_imgRect[i].left < m_imgRect[n].left)
                nXLessMore++;
            if (m_imgRect[i].top < m_imgRect[n].top)
                nYLessMore++;
        }
        if (nXLessMore > 1)
        {
            m_imgRect[n].left += nXToGrow;
            m_imgRect[n].right += nXToGrow;
        }
        if (nYLessMore > 1)
        {
            m_imgRect[n].top += nYToGrow;
            m_imgRect[n].bottom += nYToGrow;
        }
    }

    FillVertex();
}

void CXI_FOURIMAGE::SaveParametersToIni()
{
    char pcWriteParam[2048];

    auto pIni = fio->OpenIniFile(ptrOwner->m_sDialogFileName.c_str());
    if (!pIni)
    {
        core.Trace("Warning! Can`t open ini file name %s", ptrOwner->m_sDialogFileName.c_str());
        return;
    }

    // save position
    char pcWriteKeyName[256];
    for (int32_t n = 0; n < 4; n++)
    {
        sprintf_s(pcWriteParam, sizeof(pcWriteParam), "%d,%d,%d,%d", m_imgRect[n].left, m_imgRect[n].top,
                  m_imgRect[n].right, m_imgRect[n].bottom);
        sprintf_s(pcWriteKeyName, sizeof(pcWriteKeyName), "position%d", n);
        pIni->WriteString(m_nodeName, pcWriteKeyName, pcWriteParam);
    }
}

void CXI_FOURIMAGE::ChangeItem(int nItemNum)
{
    char param[256];
    auto *pAttribute = core.Entity_GetAttributeClass(g_idInterface, "FourImage");
    if (pAttribute != nullptr)
    {
        for (auto i = (nItemNum == -1 ? 0 : nItemNum); i < (nItemNum == -1 ? 4 : nItemNum + 1); i++)
        {
            if (m_pOneStr[i] != nullptr)
            {
                delete m_pOneStr[i];
                m_pOneStr[i] = nullptr;
            }
            if (m_pTwoStr[i] != nullptr)
            {
                delete m_pTwoStr[i];
                m_pTwoStr[i] = nullptr;
            }
            sprintf_s(param, "pic%d", i + 1);
            auto *pAttrTmp = pAttribute->GetAttributeClass(param);
            if (pAttrTmp != nullptr)
            {
                const char *sptr;
                m_bUsed[i] = pAttrTmp->GetAttributeAsDword("selected", 0) != 0;
                m_oneTexID[i] = pAttrTmp->GetAttributeAsDword("tex1", -1);
                m_twoTexID[i] = pAttrTmp->GetAttributeAsDword("tex2", -1);
                if (m_oneTexID[i] == -1)
                    m_oneImgID[i] = -1;
                else
                    m_oneImgID[i] =
                        pPictureService->GetImageNum(m_sGroupName[m_oneTexID[i]], pAttrTmp->GetAttribute("img1"));
                if (m_twoTexID[i] == -1)
                    m_twoImgID[i] = -1;
                else
                    m_twoImgID[i] =
                        pPictureService->GetImageNum(m_sGroupName[m_twoTexID[i]], pAttrTmp->GetAttribute("img2"));
                sptr = pAttrTmp->GetAttribute("str1");
                if (sptr != nullptr && *sptr == '#')
                {
                    const auto len = strlen(sptr);
                    if ((m_pOneStr[i] = new char[len]) == nullptr)
                    {
                        throw std::runtime_error("allocate memory error");
                    }
                    memcpy(m_pOneStr[i], &sptr[1], len);
                }
                else
                    m_oneStr[i] = pStringService->GetStringNum(sptr);

                sptr = pAttrTmp->GetAttribute("str2");
                if (sptr != nullptr && *sptr == '#')
                {
                    const auto len = strlen(sptr);
                    if ((m_pTwoStr[i] = new char[len]) == nullptr)
                    {
                        throw std::runtime_error("allocate memory error");
                    }
                    memcpy(m_pTwoStr[i], &sptr[1], len);
                }
                else
                    m_twoStr[i] = pStringService->GetStringNum(sptr);
            }
            else
            {
                m_oneImgID[i] = -1L;
                m_twoImgID[i] = -1L;
                m_oneStr[i] = -1L;
                m_twoStr[i] = -1L;
                m_pOneStr[i] = nullptr;
                m_pTwoStr[i] = nullptr;
            }
        }

        FillVertex();
    }
}

XYRECT CXI_FOURIMAGE::GetCursorRect()
{
    if (m_nSelectItem < 0 || m_nSelectItem > 3)
        return m_rect;
    return m_imgRect[m_nSelectItem];
}
