#include "xi_glower.h"

#include "math_inlines.h"

using namespace Storm::Filesystem;
using namespace Storm::Math;

CXI_GLOWER::CXI_GLOWER()
{
    m_nQuantity = 0L;
    m_texID = -1L;
    m_nNodeType = NODETYPE_GLOWER;
}

CXI_GLOWER::~CXI_GLOWER()
{
    ReleaseAll();
}

void CXI_GLOWER::Draw(bool bSelected, uint32_t Delta_Time)
{
    m_rs->TextureSet(0, m_texID);
    for (int32_t i = 0; i < m_nQuantity; i++)
    {
        if (m_glows[i].action != GLOW_ACTION_NONE)
        {
            // recalculate color
            if (m_glows[i].action == GLOW_ACTION_COLORUP)
                m_glows[i].rect.dwColor = ColorInterpolate(m_dwMaxColor, m_dwMinColor,
                                                           static_cast<float>(m_glows[i].curTime) / m_glows[i].allTime);
            if (m_glows[i].action == GLOW_ACTION_BLEND)
                m_glows[i].rect.dwColor = ColorInterpolate(
                    m_dwMaxColor, m_dwMinColor, 1.f - static_cast<float>(m_glows[i].curTime) / m_glows[i].allTime);
            // show this rectangle
            m_rs->DrawRects(&m_glows[i].rect, 1, "iGlow");

            m_glows[i].rect.fAngle += m_glows[i].angleSpeed * Delta_Time;
            while (m_glows[i].rect.fAngle > 2.f * PI)
                m_glows[i].rect.fAngle -= 2.f * PI;
            if ((m_glows[i].curTime -= Delta_Time) < 0)
            {
                switch (m_glows[i].action)
                {
                case GLOW_ACTION_COLORUP:
                    m_glows[i].action = GLOW_ACTION_SHOW;
                    m_glows[i].rect.dwColor = m_dwMaxColor;
                    m_glows[i].curTime = m_glows[i].allTime =
                        static_cast<int32_t>(m_minShowTime + (m_maxShowTime - m_minShowTime) * rand() / RAND_MAX);
                    break;
                case GLOW_ACTION_SHOW:
                    m_glows[i].action = GLOW_ACTION_BLEND;
                    m_glows[i].curTime = m_glows[i].allTime = static_cast<int32_t>(
                        m_minGlowTime + static_cast<float>(m_maxGlowTime - m_minGlowTime) * rand() / RAND_MAX);
                    break;
                case GLOW_ACTION_BLEND:
                    m_glows[i].action = GLOW_ACTION_NONE;
                    break;
                }
            }
        }
        else
        {
            // to can or not to turn on the glow
            if (rand() < m_nRandomMax)
            {
                // yes! this new glow
                m_glows[i].action = GLOW_ACTION_COLORUP;
                m_glows[i].curTime = m_glows[i].allTime = static_cast<int32_t>(
                    m_minGlowTime + static_cast<float>(m_maxGlowTime - m_minGlowTime) * rand() / RAND_MAX);
                m_glows[i].angleSpeed = -(m_fAngleSpeedMin + (m_fAngleSpeedMax - m_fAngleSpeedMin) * rand() / RAND_MAX);
            }
        }
    }
}

bool CXI_GLOWER::Init(const Config& node_config, const Config& def_config,
    VDX9RENDER *rs, XYRECT &hostRect, XYPOINT &ScreenSize)
{
    if (!CINODE::Init(node_config, def_config, rs, hostRect, ScreenSize)) {
        return false;
    }
    SetGlowCursor(false);
    return true;
}

void CXI_GLOWER::ReleaseAll()
{
    TEXTURE_RELEASE(m_rs, m_texID);
}

int CXI_GLOWER::CommandExecute(int wActCode)
{
    return -1;
}

bool CXI_GLOWER::IsClick(int buttonID, int32_t xPos, int32_t yPos)
{
    return false;
}

void CXI_GLOWER::ChangePosition(XYRECT &rNewPos)
{
    // no this action
}

void CXI_GLOWER::SaveParametersToIni()
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

void CXI_GLOWER::LoadIni(const Config& node_config, const Config& def_config) {
    // get texture
    m_texID = -1;
    std::pair<const Config&, const Config&> configs{node_config, def_config};
    const auto texture = Config::GetOrGet<std::string>(configs, "texture", {});
    if (!texture.empty()) {
        m_texID = m_rs->TextureCreate(texture.c_str());
    }

    // calculate glow blinks quantity
    const auto pos_vec = Config::GetOrGet<std::vector<Types::Vector2<std::int64_t>>>(configs, "pos", {});
    m_nQuantity = std::size(pos_vec);

    if (m_nQuantity > MAX_USED_RECTANGLE) {
        m_nQuantity = MAX_USED_RECTANGLE;
    }

    // set common default value
    for (int i = 0; i < m_nQuantity; i++) {
        m_glows[i].action = GLOW_ACTION_NONE;
        m_glows[i].rect.dwSubTexture = 0;
        m_glows[i].rect.fAngle = 0;
        m_glows[i].rect.vPos.z = 1.f;
    }

    // fill rectangle pos
    for (int i = 0; i < m_nQuantity; i++) {
        m_glows[i].rect.vPos.x = static_cast<float>(pos_vec[i].x + m_hostRect.left);
        m_glows[i].rect.vPos.y = static_cast<float>(pos_vec[i].y + m_hostRect.top);
    }

    // fill rectangles size
    const auto x = Config::GetOrGet<std::int64_t>(configs, "spriteSize", 8);
    for (int i = 0; i < m_nQuantity; i++) {
        m_glows[i].rect.fSize = static_cast<float>(x);
    }
    // set colors
    const auto min_color = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "minColor", {});
    m_dwMinColor = ARGB(min_color.x, min_color.y, min_color.z, min_color.w);
    const auto max_color = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "maxColor", {255, 255, 255, 255});
    m_dwMaxColor = ARGB(max_color.x, max_color.y, max_color.z, max_color.w);

    m_minGlowTime = Config::GetOrGet<std::int64_t>(configs, "minGlowTime", 200);
    m_maxGlowTime = Config::GetOrGet<std::int64_t>(configs, "maxGlowTime", 600);
    m_minShowTime = Config::GetOrGet<std::int64_t>(configs, "minShowTime", 200);
    m_maxShowTime = Config::GetOrGet<std::int64_t>(configs, "maxShowTime", 600);

    m_nRandomMax = static_cast<std::int32_t>(Config::GetOrGet<double>(configs, "createProbability", 0.1) * RAND_MAX);

    m_fAngleSpeedMin = Config::GetOrGet<double>(configs, "minRotateSpeed", 15.0) * PI / 180000.f;
    m_fAngleSpeedMax = Config::GetOrGet<double>(configs, "maxRotateSpeed", 180.0) * PI / 180000.f;
}
