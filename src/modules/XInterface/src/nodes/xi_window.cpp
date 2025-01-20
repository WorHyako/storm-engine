#include "xi_window.h"
#include <stdio.h>

using namespace Storm::Filesystem;

CXI_WINDOW::CXI_WINDOW()
{
    m_nNodeType = NODETYPE_WINDOW;

    m_bActive = true;
    m_bShow = true;
}

CXI_WINDOW::~CXI_WINDOW()
{
}

bool CXI_WINDOW::Init(const Config& node_config, const Config& def_config,
    VDX9RENDER *rs, XYRECT &hostRect, XYPOINT &ScreenSize) {
    if (!CINODE::Init(node_config, def_config, rs, hostRect, ScreenSize))
        return false;
    SetGlowCursor(false);
    return true;
}

void CXI_WINDOW::ChangePosition(XYRECT &rNewPos)
{
    if (m_rect.right - m_rect.left != rNewPos.right - rNewPos.left)
        rNewPos.right += rNewPos.left + m_rect.right - m_rect.left;
    if (m_rect.bottom - m_rect.top != rNewPos.bottom - rNewPos.top)
        rNewPos.bottom += rNewPos.top + m_rect.bottom - m_rect.top;
    if (rNewPos.top == m_rect.top && rNewPos.left == m_rect.left)
        return; // nothing to change - the same position
    const auto nXAdd = rNewPos.left - m_rect.left;
    const auto nYAdd = rNewPos.top - m_rect.top;
    m_rect = rNewPos;

    for (int32_t n = 0; n < m_aNodeNameList.size(); n++)
    {
        auto *pNod = ptrOwner->FindNode(m_aNodeNameList[n].c_str(), nullptr);
        if (pNod)
        {
            auto r = pNod->m_rect;
            r.left += nXAdd;
            r.right += nXAdd;
            r.top += nYAdd;
            r.bottom += nYAdd;
            pNod->ChangePosition(r);
        }
    }
}

void CXI_WINDOW::SaveParametersToIni()
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

    for (int32_t n = 0; n < m_aNodeNameList.size(); n++)
    {
        auto *pNod = ptrOwner->FindNode(m_aNodeNameList[n].c_str(), nullptr);
        if (pNod)
            pNod->SaveParametersToIni();
    }
}

void CXI_WINDOW::SetShow(bool bShow)
{
    if (bShow == m_bShow)
        return;
    if (!bShow && m_bActive)
        SetActive(false);
    m_bShow = bShow;

    // walk through all nodes and turn them on / off
    for (int32_t n = 0; n < m_aNodeNameList.size(); n++)
    {
        auto *const pNod = ptrOwner->FindNode(m_aNodeNameList[n].c_str(), nullptr);
        if (pNod)
        {
            pNod->m_bUse = bShow;
            if (pNod->m_nNodeType == NODETYPE_WINDOW)
                static_cast<CXI_WINDOW *>(pNod)->SetShow(bShow);
        }
    }
}

void CXI_WINDOW::SetActive(bool bActive)
{
    if (m_bActive == bActive)
        return;
    m_bActive = bActive;

    // pass through all nodes and lock / unlock them
    for (int32_t n = 0; n < m_aNodeNameList.size(); n++)
    {
        auto *const pNod = ptrOwner->FindNode(m_aNodeNameList[n].c_str(), nullptr);
        if (pNod)
        {
            pNod->m_bLockedNode = !bActive;
            if (pNod->m_nNodeType == NODETYPE_WINDOW)
                static_cast<CXI_WINDOW *>(pNod)->SetActive(bActive);
        }
    }
}

void CXI_WINDOW::AddNode(const char *pcNodeName)
{
    auto *pNod = ptrOwner->FindNode(pcNodeName, nullptr);
    if (!pNod)
    {
        core.Trace("Warning! CXI_WINDOW::AddNode(%s) : Node not found", pcNodeName);
        return;
    }
    m_aNodeNameList.push_back(pcNodeName);

    pNod->m_bUse = m_bShow;
    pNod->m_bLockedNode = !m_bActive;

    XYRECT r = pNod->m_rect;
    r.left += m_rect.left;
    r.right += m_rect.left;
    r.top += m_rect.top;
    r.bottom += m_rect.top;
    pNod->ChangePosition(r);
}

void CXI_WINDOW::LoadIni(const Config& node_config, const Config& def_config) {
    std::pair<const Config&, const Config&> configs{node_config, def_config};

    auto node_list_vec = node_config.Get<std::vector<std::vector<std::string>>>("nodelist", {});
    for (const auto &node : node_list_vec) {
        for (const auto& each : node) {
            m_aNodeNameList.emplace_back(std::move(each));
        }
    }

    SetActive(Config::GetOrGet<std::int64_t>(configs, "active", 1));

    SetShow(Config::GetOrGet<std::int64_t>(configs, "show", 1));
}
