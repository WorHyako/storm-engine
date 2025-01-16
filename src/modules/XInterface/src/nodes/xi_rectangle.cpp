#include "xi_rectangle.h"

using namespace Storm::Filesystem;
using namespace Storm::Math;

CXI_RECTANGLE::CXI_RECTANGLE()
{
    m_nNodeType = NODETYPE_RECTANGLE;
}

CXI_RECTANGLE::~CXI_RECTANGLE()
{
    ReleaseAll();
}

void CXI_RECTANGLE::Draw(bool bSelected, uint32_t Delta_Time)
{
    if (m_bUse)
    {
        m_rs->TextureSet(0, 0);
        m_rs->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, XI_NOTEX_FVF, 2, m_pVert, sizeof(XI_NOTEX_VERTEX), "iRectangle");
        if (m_bBorder)
        {
            RS_LINE pLines[8];
            for (auto i = 0; i < 8; i++)
            {
                pLines[i].vPos.z = 1.f;
                pLines[i].dwColor = m_dwBorderColor;
            }
            pLines[0].vPos.x = pLines[1].vPos.x = pLines[2].vPos.x = pLines[7].vPos.x = static_cast<float>(m_rect.left);
            pLines[1].vPos.y = pLines[2].vPos.y = pLines[3].vPos.y = pLines[4].vPos.y = static_cast<float>(m_rect.top);
            pLines[3].vPos.x = pLines[4].vPos.x = pLines[5].vPos.x = pLines[6].vPos.x =
                static_cast<float>(m_rect.right);
            pLines[0].vPos.y = pLines[5].vPos.y = pLines[6].vPos.y = pLines[7].vPos.y =
                static_cast<float>(m_rect.bottom);
            m_rs->DrawLines(pLines, 4, "iRectangle");
        }
    }
}

bool CXI_RECTANGLE::Init(const Config& node_config, const Config& def_config,
    VDX9RENDER *rs, XYRECT &hostRect, XYPOINT &ScreenSize) {
    return CINODE::Init(node_config, def_config, rs, hostRect, ScreenSize);
}

void CXI_RECTANGLE::ReleaseAll()
{
}

int CXI_RECTANGLE::CommandExecute(int wActCode)
{
    return -1;
}

void CXI_RECTANGLE::LoadIni(const Config& node_config, const Config& def_config) {
    std::pair<const Config&, const Config&> configs{node_config, def_config};
    // fill vertex positions
    for (auto& i : m_pVert) {
        i.pos.z = 1.f;
    }
    m_pVert[0].pos.x = static_cast<float>(m_rect.left);
    m_pVert[0].pos.y = static_cast<float>(m_rect.top);
    m_pVert[1].pos.x = static_cast<float>(m_rect.left);
    m_pVert[1].pos.y = static_cast<float>(m_rect.bottom);
    m_pVert[2].pos.x = static_cast<float>(m_rect.right);
    m_pVert[2].pos.y = static_cast<float>(m_rect.top);
    m_pVert[3].pos.x = static_cast<float>(m_rect.right);
    m_pVert[3].pos.y = static_cast<float>(m_rect.bottom);

    // Get rectangle left colors
    auto left_color = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "leftColor", {});
    m_dwLeftColor = ARGB(left_color.x, left_color.y, left_color.z, left_color.w);

    // Get rectangle top colors
    auto top_color = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "topColor", {});
    m_dwTopColor = ARGB(top_color.x, top_color.y, top_color.z, top_color.w);

    // Get rectangle right colors
    auto right_color = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "rightColor", {});
    m_dwRightColor = ARGB(right_color.x, right_color.y, right_color.z, right_color.w);

    // Get rectangle bottom colors
    auto bottom_color = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "bottomColor", {});
    m_dwBottomColor = ARGB(bottom_color.x, bottom_color.y, bottom_color.z, bottom_color.w);

    // Get bounder parameters
    auto border_color = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "borderColor", {});
    m_dwBorderColor = ARGB(border_color.x, border_color.y, border_color.z, border_color.w);
    m_bBorder = ALPHA(m_dwBorderColor) != 0;

    UpdateColors();
}

void CXI_RECTANGLE::UpdateColors()
{
    int alpha, red, green, blue;

    // set left top vertex color
    alpha = (ALPHA(m_dwLeftColor) * ALPHA(m_dwTopColor)) >> 8L;
    red = (RED(m_dwLeftColor) * RED(m_dwTopColor)) >> 8L;
    green = (GREEN(m_dwLeftColor) * GREEN(m_dwTopColor)) >> 8L;
    blue = (BLUE(m_dwLeftColor) * BLUE(m_dwTopColor)) >> 8L;
    m_pVert[0].color = ARGB(alpha, red, green, blue);

    // set left bottom vertex color
    alpha = (ALPHA(m_dwLeftColor) * ALPHA(m_dwBottomColor)) >> 8L;
    red = (RED(m_dwLeftColor) * RED(m_dwBottomColor)) >> 8L;
    green = (GREEN(m_dwLeftColor) * GREEN(m_dwBottomColor)) >> 8L;
    blue = (BLUE(m_dwLeftColor) * BLUE(m_dwBottomColor)) >> 8L;
    m_pVert[1].color = ARGB(alpha, red, green, blue);

    // set right top vertex color
    alpha = (ALPHA(m_dwRightColor) * ALPHA(m_dwTopColor)) >> 8L;
    red = (RED(m_dwRightColor) * RED(m_dwTopColor)) >> 8L;
    green = (GREEN(m_dwRightColor) * GREEN(m_dwTopColor)) >> 8L;
    blue = (BLUE(m_dwRightColor) * BLUE(m_dwTopColor)) >> 8L;
    m_pVert[2].color = ARGB(alpha, red, green, blue);

    // set right bottom vertex color
    alpha = (ALPHA(m_dwRightColor) * ALPHA(m_dwBottomColor)) >> 8L;
    red = (RED(m_dwRightColor) * RED(m_dwBottomColor)) >> 8L;
    green = (GREEN(m_dwRightColor) * GREEN(m_dwBottomColor)) >> 8L;
    blue = (BLUE(m_dwRightColor) * BLUE(m_dwBottomColor)) >> 8L;
    m_pVert[3].color = ARGB(alpha, red, green, blue);
}

bool CXI_RECTANGLE::IsClick(int buttonID, int32_t xPos, int32_t yPos)
{
    if (!m_bClickable)
        return false;
    if (xPos >= m_rect.left && xPos <= m_rect.right && yPos >= m_rect.top && yPos <= m_rect.bottom)
        return true;
    return false;
}

void CXI_RECTANGLE::ChangePosition(XYRECT &rNewPos)
{
    m_rect = rNewPos;

    m_pVert[0].pos.x = static_cast<float>(m_rect.left);
    m_pVert[0].pos.y = static_cast<float>(m_rect.top);
    m_pVert[1].pos.x = static_cast<float>(m_rect.left);
    m_pVert[1].pos.y = static_cast<float>(m_rect.bottom);
    m_pVert[2].pos.x = static_cast<float>(m_rect.right);
    m_pVert[2].pos.y = static_cast<float>(m_rect.top);
    m_pVert[3].pos.x = static_cast<float>(m_rect.right);
    m_pVert[3].pos.y = static_cast<float>(m_rect.bottom);
}

void CXI_RECTANGLE::SaveParametersToIni()
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

uint32_t CXI_RECTANGLE::MessageProc(int32_t msgcode, MESSAGE &message)
{
    switch (msgcode)
    {
    case 0: // Change the position of the rectangle
    {
        XYRECT newRect;
        newRect.left = message.Long();
        newRect.top = message.Long();
        newRect.right = message.Long();
        newRect.bottom = message.Long();
        ChangePosition(newRect);
    }
    break;
    case 1: // Change rectangle and border color
    {
        m_dwTopColor = m_dwBottomColor = ARGB(255, 255, 255, 255);
        m_dwLeftColor = m_dwRightColor = message.Long();
        m_dwBorderColor = message.Long();
        m_bBorder = ALPHA(m_dwBorderColor) != 0;
        UpdateColors();
    }
    break;
    }

    return 0;
}
