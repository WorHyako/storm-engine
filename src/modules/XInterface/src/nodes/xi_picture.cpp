#include "xi_picture.h"

#include "storm_assert.h"
#include "string_compare.hpp"
#include "file_service.h"

using namespace Storm::Filesystem;
using namespace Storm::Math;

CXI_PICTURE::CXI_PICTURE()
{
    m_rs = nullptr;
    m_idTex = -1;
    m_pTex = nullptr;
    m_nNodeType = NODETYPE_PICTURE;
    m_bMakeBlind = false;
    m_fCurBlindTime = 0.f;
    m_bBlindUp = true;
    m_fBlindUpSpeed = 0.001f;
    m_fBlindDownSpeed = 0.001f;
    m_dwBlindMin = ARGB(255, 128, 128, 128);
    m_dwBlindMax = ARGB(255, 255, 255, 255);
}

CXI_PICTURE::~CXI_PICTURE()
{
    ReleaseAll();
}

void CXI_PICTURE::Draw(bool bSelected, uint32_t Delta_Time) {
    if (!m_bUse) {
        return;
    }
    if (m_bMakeBlind) {
        if (m_bBlindUp) {
            m_fCurBlindTime += m_fBlindUpSpeed * Delta_Time;
            if (m_fCurBlindTime >= 1.f) {
                m_fCurBlindTime = 1.f;
                m_bBlindUp = false;
            }
        } else {
            m_fCurBlindTime -= m_fBlindDownSpeed * Delta_Time;
            if (m_fCurBlindTime <= 0.f) {
                m_fCurBlindTime = 0.f;
                m_bBlindUp = true;
            }
        }
        ChangeColor(ptrOwner->GetBlendColor(m_dwBlindMin, m_dwBlindMax, m_fCurBlindTime));
    }

    if (m_idTex != -1 || m_pTex) {
        if (m_idTex != -1)
            m_rs->TextureSet(0, m_idTex);
        else
            m_rs->SetTexture(0, m_pTex ? m_pTex->m_pTexture : nullptr);
        m_rs->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, XI_ONETEX_FVF, 2, m_v, sizeof(XI_ONETEX_VERTEX), "iVideo");
    }
}

bool CXI_PICTURE::Init(const Config& node_config, const Config& def_config, VDX9RENDER *rs,
                       XYRECT &hostRect, XYPOINT &ScreenSize) {
    return CINODE::Init(node_config, def_config, rs, hostRect, ScreenSize);
}

void CXI_PICTURE::LoadIni(const Config& node_config, const Config& def_config) {
    std::pair<const Config&, const Config&> configs{node_config, def_config};

    auto texRect = FXYRECT(0.f, 0.f, 1.f, 1.f);

    auto group_name = Config::GetOrGet<std::string>(configs, "groupName", {});
    if (!group_name.empty()) {
        m_idTex = pPictureService->GetTextureID(group_name.c_str());
        auto pic_name = Config::GetOrGet<std::string>(configs, "picName", {});
        if (!pic_name.empty()) {
            pPictureService->GetTexturePos(m_pcGroupName.c_str(), pic_name.c_str(), texRect);
        }
    } else {
        auto texture_name = Config::GetOrGet<std::string>(configs, "textureName", {});
        m_idTex = texture_name.empty()
            ? -1
            : m_rs->TextureCreate(texture_name.c_str());
        auto texture_rect_opt = Config::GetOrGet<Types::Vector4<double>>(configs, "textureRect");
        if (texture_rect_opt) {
            texRect = *texture_rect_opt;
        }
    }

    auto video_name = Config::GetOrGet<std::string>(configs, "videoName", {});
    m_pTex = video_name.empty()
        ? nullptr
        : m_rs->GetVideoTexture(video_name.c_str());

    auto color_vec = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "color", {255, 128, 128, 128});
    auto color = ARGB(color_vec.x, color_vec.y, color_vec.z, color_vec.w);

    // Create rectangle
    ChangePosition(m_rect);
    ChangeUV(texRect);
    for (auto i = 0; i < 4; i++) {
        m_v[i].color = color;
        m_v[i].pos.z = 1.f;
    }

    m_bMakeBlind = Config::GetOrGet<std::int64_t>(configs, "blind", false);
    m_fCurBlindTime = 0.f;
    m_bBlindUp = true;
    auto fTmp = Config::GetOrGet<double>(configs, "blindUpTime", 1.0);
    if (fTmp > 0.f) {
        m_fBlindUpSpeed = 0.001f / fTmp;
    }
    fTmp = Config::GetOrGet<double>(configs, "blindDownTime", 1.0);
    if (fTmp > 0.f)
        m_fBlindDownSpeed = 0.001f / fTmp;
    auto blind_min = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "blindMinColor", {255, 128, 128, 128});
    m_dwBlindMin = ARGB(blind_min.x, blind_min.y, blind_min.z, blind_min.w);

    auto blind_max = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "blindMaxColor", {255});
    m_dwBlindMin = ARGB(blind_max.x, blind_max.y, blind_max.z, blind_max.w);
}

void CXI_PICTURE::ReleaseAll()
{
    ReleasePicture();
}

int CXI_PICTURE::CommandExecute(int wActCode)
{
    return -1;
}

bool CXI_PICTURE::IsClick(int buttonID, int32_t xPos, int32_t yPos)
{
    if (m_bClickable)
    {
        if (xPos >= m_rect.left && xPos <= m_rect.right && yPos >= m_rect.top && yPos <= m_rect.bottom)
            return true;
    }
    return false;
}

void CXI_PICTURE::ChangePosition(XYRECT &rNewPos)
{
    m_rect = rNewPos;
    m_v[0].pos.x = static_cast<float>(m_rect.left);
    m_v[0].pos.y = static_cast<float>(m_rect.top);
    m_v[1].pos.x = static_cast<float>(m_rect.left);
    m_v[1].pos.y = static_cast<float>(m_rect.bottom);
    m_v[2].pos.x = static_cast<float>(m_rect.right);
    m_v[2].pos.y = static_cast<float>(m_rect.top);
    m_v[3].pos.x = static_cast<float>(m_rect.right);
    m_v[3].pos.y = static_cast<float>(m_rect.bottom);
}

void CXI_PICTURE::SaveParametersToIni()
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

void CXI_PICTURE::SetNewPicture(bool video, const char *sNewTexName)
{
    ReleasePicture();
    if (video)
        m_pTex = m_rs->GetVideoTexture(sNewTexName);
    else
        m_idTex = m_rs->TextureCreate(sNewTexName);

    FXYRECT uv;
    uv.left = uv.top = 0.f;
    uv.right = uv.bottom = 1.f;
    ChangeUV(uv);
}

void CXI_PICTURE::SetNewPictureFromDir(const char *dirName)
{
    char param[512];
    sprintf(param, "resource\\textures\\%s", dirName);

    const auto vFilenames = fio->_GetPathsOrFilenamesByMask(param, "*.tx", false);
    if (!vFilenames.empty())
    {
        int findQ = rand() % vFilenames.size();
        sprintf(param, "%s\\%s", dirName, vFilenames[findQ].c_str());
        const int paramlen = strlen(param);
        if (paramlen < sizeof(param) && paramlen >= 3)
        {
            param[paramlen - 3] = 0;
        }
        SetNewPicture(false, param);
    }
}

void CXI_PICTURE::SetNewPictureByGroup(const char *groupName, const char *picName) {
    if (m_pcGroupName.empty() || !storm::iEquals(m_pcGroupName, groupName)) {
        ReleasePicture();
        if (groupName) {
            m_pcGroupName = std::string(groupName);
            m_idTex = pPictureService->GetTextureID(groupName);
        }
    }

    if (!m_pcGroupName.empty() && picName) {
        FXYRECT texRect;
        pPictureService->GetTexturePos(m_pcGroupName.c_str(), picName, texRect);
        ChangeUV(texRect);
    }
}

uint32_t CXI_PICTURE::MessageProc(int32_t msgcode, MESSAGE &message)
{
    switch (msgcode)
    {
    case 0: // Move the picture to a new position
    {
        m_rect.left = message.Long();
        m_rect.top = message.Long();
        m_rect.right = message.Long();
        m_rect.bottom = message.Long();
        ChangePosition(m_rect);
    }
    break;

    case 1: // Set the texture coordinates of the image
    {
        FXYRECT texRect;
        texRect.left = message.Float();
        texRect.right = message.Float();
        texRect.top = message.Float();
        texRect.bottom = message.Float();
        ChangeUV(texRect);
    }
    break;

    case 2: // Set a new picture or video picture
    {
        const auto bVideo = message.Long() != 0;
        const std::string &param = message.String();
        SetNewPicture(bVideo, param.c_str());
    }
    break;

    case 3: // Get a random picture from the directory
    {
        const std::string &param = message.String();
        SetNewPictureFromDir(param.c_str());
    }
    break;

    case 4: // Set a new color
    {
        const uint32_t color = message.Long();
        for (auto i = 0; i < 4; i++)
            m_v[i].color = color;
    }
    break;

    case 5: // set / remove blinking
    {
        const bool bBlind = message.Long() != 0;
        if (m_bMakeBlind != bBlind)
        {
            m_bMakeBlind = bBlind;
            if (!m_bMakeBlind)
                ChangeColor(m_dwBlindMin);
            else
            {
                m_fCurBlindTime = 0.f;
                m_bBlindUp = true;
            }
        }
    }
    break;

    case 6: // set new picture by group and picture name
    {
        const std::string &groupName = message.String();
        const std::string &picName = message.String();
        SetNewPictureByGroup(groupName.c_str(), picName.c_str());
    }
    break;

    case 7: // set new picture by pointer to IDirect3DTexture9
    {
        int32_t pTex = -1;
        if (message.GetCurrentFormatType() == 'p') {
            // DEPRECATED
            core.Trace("Warning! Setting an interface picture by pointer is deprecated. Please use integers instead.");
            pTex = message.Pointer();
        }
        else {
            pTex = message.Long();
        }
        SetNewPictureByPointer(pTex);
    }
    break;

    case 8: // remove texture from other picture to this
    {
        const std::string &srcNodeName = message.String();
        auto *pNod = static_cast<CINODE *>(ptrOwner->FindNode(srcNodeName.c_str(), nullptr));
        if (pNod->m_nNodeType != NODETYPE_PICTURE)
        {
            core.Trace("Warning! XINTERFACE:: node with name %s have not picture type.", srcNodeName.c_str());
        }
        else
        {
            ReleasePicture();
            auto *pOtherPic = static_cast<CXI_PICTURE *>(pNod);
            if (!pOtherPic->m_pcGroupName.empty())
            {
                m_pcGroupName = pOtherPic->m_pcGroupName;
            }
            if (pOtherPic->m_idTex != -1)
            {
                m_idTex = pOtherPic->m_idTex;
                pOtherPic->m_idTex = -1;
            }
            for (int32_t n = 0; n < 4; n++)
            {
                m_v[n].tu = pOtherPic->m_v[n].tu;
                m_v[n].tv = pOtherPic->m_v[n].tv;
            }
            pOtherPic->ReleasePicture();
        }
    }
    break;
    }

    return 0;
}

void CXI_PICTURE::ChangeUV(FXYRECT &frNewUV)
{
    m_v[0].tu = frNewUV.left;
    m_v[0].tv = frNewUV.top;
    m_v[1].tu = frNewUV.left;
    m_v[1].tv = frNewUV.bottom;
    m_v[2].tu = frNewUV.right;
    m_v[2].tv = frNewUV.top;
    m_v[3].tu = frNewUV.right;
    m_v[3].tv = frNewUV.bottom;
}

void CXI_PICTURE::ChangeColor(uint32_t dwColor)
{
    m_v[0].color = m_v[1].color = m_v[2].color = m_v[3].color = dwColor;
}

void CXI_PICTURE::SetPictureSize(int32_t &nWidth, int32_t &nHeight)
{
    if (!m_pTex && m_idTex == -1)
    {
        m_bUse = false;
        nWidth = nHeight = 0;
        return;
    }

    if (nWidth <= 0)
    {
        // find the real width
        nWidth = 128;
    }
    if (nHeight <= 0)
    {
        // find the real height
        nHeight = 128;
    }

    if (nWidth < 0 || nHeight < 0)
    {
        m_bUse = false;
        nWidth = nHeight = 0;
        return;
    }

    XYRECT rNewPos = m_rect;
    if (rNewPos.right - rNewPos.left != nWidth)
    {
        rNewPos.left = (m_rect.left + m_rect.right - nWidth) / 2;
        rNewPos.right = rNewPos.left + nWidth;
    }
    if (rNewPos.bottom - rNewPos.top != nHeight)
    {
        rNewPos.top = (m_rect.top + m_rect.bottom - nHeight) / 2;
        rNewPos.bottom = rNewPos.top + nHeight;
    }
    ChangePosition(rNewPos);
}

void CXI_PICTURE::SetNewPictureByPointer(int32_t textureId)
{
    IDirect3DBaseTexture9 *texture = m_rs->GetTextureFromID(textureId);
    m_rs->TextureIncReference(textureId);
    if (texture)
        texture->AddRef();
    ReleasePicture();
    m_idTex = textureId;

    FXYRECT uv;
    uv.left = uv.top = 0.f;
    uv.right = uv.bottom = 1.f;
    ChangeUV(uv);
}

void CXI_PICTURE::ReleasePicture()
{
    PICTURE_TEXTURE_RELEASE(pPictureService, m_pcGroupName.c_str(), m_idTex);

    TEXTURE_RELEASE(m_rs, m_idTex);
    VIDEOTEXTURE_RELEASE(m_rs, m_pTex);
}
