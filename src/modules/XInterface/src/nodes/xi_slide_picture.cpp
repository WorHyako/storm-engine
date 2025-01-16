#include "xi_slide_picture.h"

#include <stdio.h>

using namespace Storm::Filesystem;
using namespace Storm::Math;

void SetTextureCoordinate(XI_ONETEX_VERTEX v[4], FXYRECT tr, float angle)
{
    if (angle == 0)
    {
        v[0].tu = v[1].tu = tr.left;
        v[2].tu = v[3].tu = tr.right;
        v[0].tv = v[2].tv = tr.top;
        v[1].tv = v[3].tv = tr.bottom;
    }
    else
    {
        const auto x = (tr.left + tr.right) * .5f;
        const auto y = (tr.top + tr.bottom) * .5f;
        const auto width = tr.right - tr.left;
        const auto height = tr.bottom - tr.top;
        const auto ca = cosf(angle);
        const auto sa = sinf(angle);
        const auto wca = width / 2 * ca;
        const auto wsa = width / 2 * sa;
        const auto hca = height / 2 * ca;
        const auto hsa = height / 2 * sa;
        v[0].tu = x + (-wca + hsa);
        v[0].tv = y + (-wsa - hca);
        v[1].tu = x + (-wca - hsa);
        v[1].tv = y + (-wsa + hca);
        v[2].tu = x + (wca + hsa);
        v[2].tv = y + (wsa - hca);
        v[3].tu = x + (wca - hsa);
        v[3].tv = y + (wsa + hca);
    }
}

CXI_SLIDEPICTURE::CXI_SLIDEPICTURE()
    : m_v{}, minRotate(0), deltaRotate(0), curRotate(0), curAngle(0), nCurSlide(0)
{
    nLifeTime = 0;
    m_rs = nullptr;
    m_idTex = -1L;
    m_nNodeType = NODETYPE_SLIDEPICTURE;
    pSlideSpeedList = nullptr;
    nSlideListSize = 0;
}

CXI_SLIDEPICTURE::~CXI_SLIDEPICTURE() {
    ReleaseAll();
}

void CXI_SLIDEPICTURE::Draw(bool bSelected, uint32_t Delta_Time) {
    if (!m_bUse) {
        return;
    }
    Update(Delta_Time);
    m_rs->TextureSet(0, m_idTex);
    m_rs->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    m_rs->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    m_rs->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, XI_ONETEX_FVF, 2, m_v, sizeof(XI_ONETEX_VERTEX),
        strTechniqueName.empty()
            ? "iVideo"
            : strTechniqueName.c_str());
}

bool CXI_SLIDEPICTURE::Init(const Config& node_config, const Config& def_config,
    VDX9RENDER *rs, XYRECT &hostRect, XYPOINT &ScreenSize) {
    if (!CINODE::Init(node_config, def_config, rs, hostRect, ScreenSize)) {
        return false;
    }
    SetGlowCursor(false);
    return true;
}

void CXI_SLIDEPICTURE::LoadIni(const Config& node_config, const Config& def_config) {
    std::pair<const Config&, const Config&> configs{node_config, def_config};
    strTechniqueName = Config::GetOrGet<std::string>(configs, "techniqueName", {});

    auto texture_name = Config::GetOrGet<std::string>(configs, "textureName", {});
    if (!texture_name.empty()) {
        m_idTex = m_rs->TextureCreate(texture_name.c_str());
    }

    m_texRect = Config::GetOrGet<Types::Vector4<double>>(configs, "textureRect", {0.0, 0.0, 1.0, 1.0});

    auto color_vec = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "color", {255, 255, 255, 255});
    const auto color = ARGB(color_vec.x, color_vec.y, color_vec.z, color_vec.w);

    // Create rectangle
    m_v[0].pos.x = static_cast<float>(m_rect.left);
    m_v[0].pos.y = static_cast<float>(m_rect.top);
    m_v[0].tu = m_texRect.left;
    m_v[0].tv = m_texRect.top;
    m_v[1].pos.x = static_cast<float>(m_rect.left);
    m_v[1].pos.y = static_cast<float>(m_rect.bottom);
    m_v[1].tu = m_texRect.left;
    m_v[1].tv = m_texRect.bottom;
    m_v[2].pos.x = static_cast<float>(m_rect.right);
    m_v[2].pos.y = static_cast<float>(m_rect.top);
    m_v[2].tu = m_texRect.right;
    m_v[2].tv = m_texRect.top;
    m_v[3].pos.x = static_cast<float>(m_rect.right), m_v[3].pos.y = static_cast<float>(m_rect.bottom);
    m_v[3].tu = m_texRect.right;
    m_v[3].tv = m_texRect.bottom;
    for (int i = 0; i < 4; i++) {
        m_v[i].color = color;
        m_v[i].pos.z = 1.f;
    }

    curAngle = 0.f;
    curRotate = 0.f;
    FXYPOINT fPos;
    fPos = Config::GetOrGet<Types::Vector2<double>>(configs, "rotate", {});
    minRotate = fPos.x;
    deltaRotate = fPos.y;

    nLifeTime = 0;
    nCurSlide = 0;
    nSlideListSize = 0;
    pSlideSpeedList = nullptr;

    bool bUse1Ini{true};

    auto speed_vec = Config::GetOrGet<std::vector<std::string>>(configs, "speed", {});
    nSlideListSize = std::size(speed_vec);

    if (nSlideListSize > 0) {
        pSlideSpeedList = new SLIDE_SPEED[nSlideListSize];
        if (pSlideSpeedList == nullptr) {
            throw std::runtime_error("allocate memory error");
        }
    }

    for (int i = 0; i < nSlideListSize; i++) {
        pSlideSpeedList[i].time = 0;
        pSlideSpeedList[i].xspeed = 0;
        pSlideSpeedList[i].yspeed = 0;
        GetDataStr(speed_vec[i].c_str(), "lff", &pSlideSpeedList[i].time, &pSlideSpeedList[i].xspeed, &pSlideSpeedList[i].yspeed);
    }
}

void CXI_SLIDEPICTURE::ReleaseAll()
{
    TEXTURE_RELEASE(m_rs, m_idTex);
    STORM_DELETE(pSlideSpeedList);
    nSlideListSize = 0;
}

int CXI_SLIDEPICTURE::CommandExecute(int wActCode)
{
    return -1;
}

bool CXI_SLIDEPICTURE::IsClick(int buttonID, int32_t xPos, int32_t yPos)
{
    return false;
}

void CXI_SLIDEPICTURE::ChangePosition(XYRECT &rNewPos)
{
    m_rect = rNewPos;
    m_v[0].pos.x = static_cast<float>(m_rect.left);
    m_v[0].pos.y = static_cast<float>(m_rect.top);
    m_v[1].pos.x = static_cast<float>(m_rect.left);
    m_v[1].pos.y = static_cast<float>(m_rect.bottom);
    m_v[2].pos.x = static_cast<float>(m_rect.right);
    m_v[2].pos.y = static_cast<float>(m_rect.top);
    m_v[3].pos.x = static_cast<float>(m_rect.right), m_v[3].pos.y = static_cast<float>(m_rect.bottom);
}

void CXI_SLIDEPICTURE::SaveParametersToIni()
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

void CXI_SLIDEPICTURE::SetNewPicture(char *sNewTexName)
{
    if (m_idTex != -1L)
        m_rs->TextureRelease(m_idTex);
    m_idTex = m_rs->TextureCreate(sNewTexName);
}

void CXI_SLIDEPICTURE::Update(uint32_t Delta_Time)
{
    if (nCurSlide >= nSlideListSize)
        return;

    nLifeTime -= Delta_Time;
    if (nLifeTime < 0)
    {
        // changing speed
        nCurSlide++;
        if (nCurSlide >= nSlideListSize)
            nCurSlide = 0;
        nLifeTime = pSlideSpeedList[nCurSlide].time;
        curRotate = minRotate + rand() * deltaRotate / RAND_MAX;
    }

    const auto xadd = pSlideSpeedList[nCurSlide].xspeed * (Delta_Time / 1000.f);
    const auto yadd = pSlideSpeedList[nCurSlide].yspeed * (Delta_Time / 1000.f);

    curAngle += curRotate * Delta_Time / 1000.f;

    m_texRect.left += xadd;
    m_texRect.right += xadd;
    m_texRect.top += yadd;
    m_texRect.bottom += yadd;

    while (m_texRect.left < -10)
    {
        m_texRect.left += 10;
        m_texRect.right += 10;
    }
    while (m_texRect.right > 10)
    {
        m_texRect.left -= 10;
        m_texRect.right -= 10;
    }
    while (m_texRect.top < -10)
    {
        m_texRect.top += 10;
        m_texRect.bottom += 10;
    }
    while (m_texRect.top > 10)
    {
        m_texRect.top -= 10;
        m_texRect.bottom -= 10;
    }

    SetTextureCoordinate(m_v, m_texRect, curAngle);
}
