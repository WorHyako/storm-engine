#include "xi_line_collection.h"

using namespace Storm::Filesystem;

CXI_LINECOLLECTION::CXI_LINECOLLECTION()
{
    m_rs = nullptr;
    m_nNodeType = NODETYPE_LINECOLLECTION;
}

CXI_LINECOLLECTION::~CXI_LINECOLLECTION()
{
    ReleaseAll();
}

int CXI_LINECOLLECTION::CommandExecute(int wActCode)
{
    return -1;
}

void CXI_LINECOLLECTION::Draw(bool bSelected, uint32_t Delta_Time)
{
    if (m_bUse)
    {
        m_rs->DrawLines(m_aLines.data(), m_aLines.size() / 2, "iLineCollection");
    }
}

bool CXI_LINECOLLECTION::Init(const Config& node_config, const Config& def_config,
    VDX9RENDER *rs, XYRECT &hostRect, XYPOINT &ScreenSize) {
    if (!CINODE::Init(node_config, def_config, rs, hostRect, ScreenSize))
        return false;
    // screen position for that is host screen position
    memcpy(&m_rect, &m_hostRect, sizeof(m_hostRect));
    SetGlowCursor(false);
    return true;
}

void CXI_LINECOLLECTION::LoadIni(const Config& node_config, const Config& def_config) {
    // fill lines structure array
    std::pair<const Config&, const Config&> configs{node_config, def_config};
    auto bRelativeRect = Config::GetOrGet<std::int64_t>(configs, "bAbsoluteRectangle", 0) == false;
    auto line_vec = Config::GetOrGet<std::vector<std::vector<std::string>>>(configs, "line", {});
    for (auto & line : line_vec) {
        std::stringstream ss;
        std::ranges::for_each(line,
            [&ss](auto& each) {
                ss << each << ',';
        });
        std::string str {ss.str()};
        XYRECT scrRect;
        uint32_t dwCol{0};
        std::size_t idx_begin{0}, idx_end{0}, res{0};

        idx_begin = str.find("col:{");
        idx_end = str.find('}', idx_begin);
        if (idx_begin != std::string::npos && idx_end != std::string::npos) {
            idx_begin += 5;
            int a{}, r{}, g{}, b{};
            res = sscanf_s(str.substr(idx_begin, idx_end - idx_begin).c_str(), "%d,%d,%d,%d", &a, &r, &g, &b);
            if (res == 4) {
                dwCol = ARGB(a, r, g, b);
            }
        }

        idx_begin = str.find('(');
        idx_end = str.find(")-(", idx_begin);
        if (idx_begin != std::string::npos && idx_end != std::string::npos) {
            int x{0}, y{0};
            idx_begin += 1;
            res = sscanf_s(str.substr(idx_begin, idx_end - idx_begin).c_str(), "%d,%d", &x, &y);
            if (res == 2) {
                scrRect.left = x;
                scrRect.top = y;
            }
        }

        idx_begin = str.find(")-(");
        idx_end = str.find('(', idx_begin);
        if (idx_begin != std::string::npos && idx_end != std::string::npos) {
            int x{0}, y{0};
            idx_begin += 3;
            res = sscanf_s(str.substr(idx_begin, idx_end - idx_begin).c_str(), "%d,%d", &x, &y);
            if (res == 2) {
                scrRect.right = x;
                scrRect.bottom = y;
            }
        }
        if (bRelativeRect) {
            GetRelativeRect(scrRect);
        }
        // int32_t n = m_aLines.Add();
        // m_aLines.Add();
        // m_aLines[n].dwColor = m_aLines[n+1].dwColor = dwCol;
        // m_aLines[n].vPos.z = m_aLines[n+1].vPos.z = 1.f;
        // m_aLines[n].vPos.x = (float)scrRect.left; m_aLines[n+1].vPos.x = (float)scrRect.right;
        // m_aLines[n].vPos.y = (float)scrRect.top;  m_aLines[n+1].vPos.y = (float)scrRect.bottom;
        m_aLines.push_back(
            RS_LINE{CVECTOR{static_cast<float>(scrRect.left), static_cast<float>(scrRect.top), 1.f}, dwCol});
        m_aLines.push_back(
            RS_LINE{CVECTOR{static_cast<float>(scrRect.right), static_cast<float>(scrRect.bottom), 1.f}, dwCol});
    }
}

void CXI_LINECOLLECTION::ReleaseAll()
{
    m_aLines.clear();
}

bool CXI_LINECOLLECTION::IsClick(int buttonID, int32_t xPos, int32_t yPos)
{
    return false;
}

void CXI_LINECOLLECTION::ChangePosition(XYRECT &rNewPos)
{
    // no this action
}

void CXI_LINECOLLECTION::SaveParametersToIni()
{
    //    char pcWriteParam[2048];

    auto pIni = fio->OpenIniFile(ptrOwner->m_sDialogFileName.c_str());
    if (!pIni)
    {
        core.Trace("Warning! Can`t open ini file name %s", ptrOwner->m_sDialogFileName.c_str());
        return;
    }

    // save position
    //    sprintf_s( pcWriteParam, sizeof(pcWriteParam), "%d,%d,%d,%d", m_rect.left, m_rect.top, m_rect.right,
    // m_rect.bottom );     pIni->WriteString( m_nodeName, "position", pcWriteParam );
}

uint32_t CXI_LINECOLLECTION::MessageProc(int32_t msgcode, MESSAGE &message)
{
    switch (msgcode)
    {
    case 0: // change color for line with number or all lines (if number = -1)
    {
        const uint32_t dwColor = message.Long();
        const auto nLineNum = message.Long();
        if (nLineNum < 0 || nLineNum >= static_cast<int32_t>(m_aLines.size()) / 2)
        {
            for (int32_t n = 0; n < m_aLines.size(); n++)
                m_aLines[n].dwColor = dwColor;
        }
        else
        {
            m_aLines[nLineNum * 2].dwColor = m_aLines[nLineNum * 2 + 1].dwColor = dwColor;
        }
    }
    break;
    case 1: // add line and return its number
    {
        const uint32_t dwColor = message.Long();
        const auto nLeft = message.Long();
        const auto nTop = message.Long();
        const auto nRight = message.Long();
        const int32_t nBottom = message.Long();
        // int32_t nLineNum = m_aLines.Add() / 2;
        // m_aLines.Add();
        const int32_t nLineNum = m_aLines.size() / 2;
        m_aLines.resize(m_aLines.size() + 2);
        m_aLines[nLineNum * 2].dwColor = m_aLines[nLineNum * 2 + 1].dwColor = dwColor;
        m_aLines[nLineNum * 2].vPos.z = m_aLines[nLineNum * 2 + 1].vPos.z = 1.f;
        m_aLines[nLineNum * 2].vPos.x = static_cast<float>(nLeft);
        m_aLines[nLineNum * 2 + 1].vPos.x = static_cast<float>(nRight);
        m_aLines[nLineNum * 2].vPos.y = static_cast<float>(nTop);
        m_aLines[nLineNum * 2 + 1].vPos.y = static_cast<float>(nBottom);
        return nLineNum;
    }
    break;
    }
    return 0;
}
