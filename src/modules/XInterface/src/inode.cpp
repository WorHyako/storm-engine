#include "inode.h"
#include <cstdarg>

#include "core.h"
#include "string_compare.hpp"

#include "Filesystem/Config/Config.hpp"

using namespace Storm::Filesystem;
using namespace Storm::Math;

CINODE::CINODE()
{
    m_bMouseWeelReaction = false;
    m_bLockedNode = false;
    m_nDoDelay = 0;
    m_nCurrentCommandNumber = -1;
    m_bUse = true;
    m_next = nullptr;
    m_list = nullptr;
    m_bClickable = false;
    m_bSelected = false;
    m_bLockStatus = false;
    m_bBreakPress = false;
    m_bMouseSelect = false;
    m_nodeName = nullptr;
    m_bShowGlowCursor = true;
    m_bUseUserGlowCursor = false;
    m_bUseUserGlowOffset = false;
    m_bInProcessingMessageForThisNode = false;
    m_bDeleting = false;
    m_pToolTip = nullptr;
    m_bMakeActionInDeclick = false;
}

CINODE::~CINODE()
{
    STORM_DELETE(m_nodeName);

    if (m_list)
    {
        m_list->ReleaseAll();
        delete m_list;
    }

    for (auto i = 0; i < COMMAND_QUANTITY; i++)
    {
        STORM_DELETE(m_pCommands[i].sRetControl);
        STORM_DELETE(m_pCommands[i].sEventName);

        auto *pContrl = m_pCommands[i].pNextControl;
        while (pContrl != nullptr)
        {
            auto *const pOld = pContrl;
            pContrl = pContrl->next;
            delete pOld;
        }
        m_pCommands[i].pNextControl = nullptr;
    }
    STORM_DELETE(m_pToolTip);
}

void CINODE::FrameProcess(uint32_t DeltaTime)
{
    if (m_nCurrentCommandNumber != -1)
    {
        m_nDoDelay -= DeltaTime;
        if (m_nDoDelay < 0)
            m_nDoDelay = 0;

        if (m_nDoDelay == 0)
        {
            // redirect command to subnodes
            auto *pContrl = m_pCommands[m_nCurrentCommandNumber].pNextControl;
            while (pContrl != nullptr)
            {
                if (pContrl->sControlName)
                {
                    auto *pTmpNod = ptrOwner->FindNode(pContrl->sControlName, nullptr);
                    if (pTmpNod)
                        pTmpNod->CommandExecute(pContrl->command);
                }
                pContrl = pContrl->next;
            }

            if (m_pCommands[m_nCurrentCommandNumber].sEventName != nullptr)
                core.Send_Message(g_idInterface, "lssl", MSG_INTERFACE_SET_EVENT,
                                  m_pCommands[m_nCurrentCommandNumber].sEventName, m_nodeName, m_nCurrentCommandNumber);

            if (m_pCommands[m_nCurrentCommandNumber].sRetControl)
            {
                auto *const pTmpNod = ptrOwner->FindNode(m_pCommands[m_nCurrentCommandNumber].sRetControl, nullptr);
                if (pTmpNod)
                    core.Send_Message(g_idInterface, "lp", MSG_INTERFACE_SET_CURRENT_NODE, pTmpNod);
            }

            m_nCurrentCommandNumber = -1;
        }
    }

    // tooltip update
    if (m_pToolTip && m_bUse)
    {
        m_pToolTip->Draw();
    }
}

CINODE *CINODE::DoAction(int wActCode, bool &bBreakPress, bool bFirstPress)
{
    if (m_nNodeType == NODETYPE_TEXTBUTTON && !m_bSelected)
        return nullptr;
    bBreakPress = m_bBreakPress;
    if (m_bLockStatus)
        return this;

    int i;
    for (i = 0; i < COMMAND_QUANTITY; i++)
        if (pCommandsList[i].code == wActCode)
            break;
    if (i == COMMAND_QUANTITY)
        return this;

    auto n = i;
    if (m_pCommands[i].bUse)
    {
        // if(m_pCommands[i].nSound!=0)
        if (bFirstPress)
            core.Event(ISOUND_EVENT, "l", 1);
        // core.Event(ISOUND_EVENT,"l",m_pCommands[i].nSound);
        // execute command
        while (n != COMMAND_QUANTITY)
        {
            const auto ac = CommandExecute(pCommandsList[n].code);
            if (ac == -1)
                break;

            for (n = 0; n < COMMAND_QUANTITY; n++)
                if (pCommandsList[n].code == ac)
                    break;
        }
        m_nDoDelay = m_pCommands[i].nActionDelay;
        if (n < COMMAND_QUANTITY)
        {
            core.Event("ievnt_command", "ss", pCommandsList[n].sName, m_nodeName);
            m_nCurrentCommandNumber = n;
        }
        else
        {
            core.Event("ievnt_command", "ss", pCommandsList[i].sName, m_nodeName);
            m_nCurrentCommandNumber = i;
        }
    }

    FrameProcess(0);

    // return other or self node
    if (m_nCurrentCommandNumber == -1)
    {
        if (n < COMMAND_QUANTITY)
            return ptrOwner->FindNode(m_pCommands[n].sRetControl, nullptr);
        return ptrOwner->FindNode(m_pCommands[i].sRetControl, nullptr);
    }
    return nullptr;
}

CINODE *CINODE::FindNode(CINODE *pNod, const char *sNodName)
{
    if (!sNodName)
        return nullptr;
    while (pNod)
    {
        if (pNod->m_nodeName && storm::iEquals(sNodName, pNod->m_nodeName))
            break;
        if (pNod->m_list)
        {
            auto *const pInsideNod = FindNode(pNod->m_list, sNodName);
            if (pInsideNod)
                return pInsideNod;
        }
        pNod = pNod->m_next;
    }
    return pNod;
}

CINODE *CINODE::FindNode(CINODE *pNod, int nNodType)
{
    while (pNod)
    {
        if (pNod->m_nNodeType == nNodType)
            break;
        if (pNod->m_list)
        {
            auto *const pInsideNod = FindNode(pNod->m_list, nNodType);
            if (pInsideNod)
                return pInsideNod;
        }
        pNod = pNod->m_next;
    }
    return pNod;
}

CINODE *CINODE::FindNode(CINODE *pNod, float x, float y)
{
    while (pNod)
    {
        if ((x >= pNod->m_rect.left) && (x <= pNod->m_rect.right) && (y >= pNod->m_rect.top) &&
            (y <= pNod->m_rect.bottom))
            break;
        if (pNod->m_list)
        {
            auto *const pInsideNod = FindNode(pNod->m_list, x, y);
            if (pInsideNod)
                return pInsideNod;
        }
        pNod = pNod->m_next;
    }
    return pNod;
}

/*CINODE*    CINODE::FindNode(const char* sNodName)
{
    if( sNodName!=null && storm::iEquals(sNodName,m_nodeName))
        return this;
    CINODE* retVal;
    if( m_list!=null )
        if( (retVal=m_list->FindNode(sNodName))!=null ) return retVal;
    if( m_next!=null ) return m_next->FindNode(sNodName);
    return null;
}

CINODE* CINODE::FindNode(int nNodType)
{
    if(m_nNodeType==nNodType)
        return this;
    CINODE* retVal;
    if( m_list!=null )
        if( (retVal=m_list->FindNode(nNodType))!=null ) return retVal;
    if( m_next!=null ) return m_next->FindNode(nNodType);
    return null;
}

CINODE* CINODE::FindNode(float x,float y)
{
    if( !m_bLockedNode )
    {
        if( (x>=m_rect.left) && (x<=m_rect.right) && (y>=m_rect.top) && (y<=m_rect.bottom) )
            return this;
        CINODE* retVal;
        if( m_list!=null )
            if( (retVal=m_list->FindNode(x,y))!=null ) return retVal;
    }
    if( m_next!=null ) return m_next->FindNode(x,y);
    return null;
}*/

void CINODE::GetRelativeRect(XYRECT &rect) const
{
    rect.left += m_hostRect.left;
    rect.top += m_hostRect.top;
    rect.right += m_hostRect.left;
    rect.bottom += m_hostRect.top;
}

void CINODE::GetAbsoluteRect(XYRECT &rect, int at) const
{
    if (!(at & ABSOLUTE_LEFT))
        rect.left += m_hostRect.left;
    if (at & ABSOLUTE_RIGHT)
        rect.right += m_screenSize.x - m_hostRect.right + m_hostRect.left;
    else
        rect.right += m_hostRect.left;

    if (!(at & ABSOLUTE_TOP))
        rect.top += m_hostRect.top;
    if (at & ABSOLUTE_BOTTOM)
        rect.bottom += m_screenSize.y - m_hostRect.bottom + m_hostRect.top;
    else
        rect.bottom += m_hostRect.top;
}

void CINODE::GetAbsoluteRectForSave(XYRECT &rect, int at) const
{
    if (!(at & ABSOLUTE_LEFT))
        rect.left -= m_hostRect.left;
    if (at & ABSOLUTE_RIGHT)
        rect.right -= m_screenSize.x - m_hostRect.right + m_hostRect.left;
    else
        rect.right -= m_hostRect.left;

    if (!(at & ABSOLUTE_TOP))
        rect.top -= m_hostRect.top;
    if (at & ABSOLUTE_BOTTOM)
        rect.bottom -= m_screenSize.y - m_hostRect.bottom + m_hostRect.top;
    else
        rect.bottom -= m_hostRect.top;
}

bool CINODE::CheckByToolTip(float fX, float fY)
{
    if (m_pToolTip)
    {
        m_pToolTip->MousePos(core.GetDeltaTime() * .001f, static_cast<int32_t>(fX), static_cast<int32_t>(fY));
        return true;
    }
    return false;
}

void CINODE::ShowToolTip() const
{
    if (m_pToolTip)
        m_pToolTip->Draw();
}

uint32_t CINODE::MessageProc(int32_t msgcode, MESSAGE &message)
{
    switch (msgcode)
    {
    case 0: // Execute node command
    {
        const auto commCode = message.Long();
        CommandExecute(commCode);
    }
    break;

    case 1: // Set clickable status
    {
        const auto clickState = message.Long();
        m_bClickable = clickState != 0;
    }
    break;

    case 2: // Get clickable status
        return m_bClickable ? 1 : 0;
        break;

    case 3: // Execute node command for command name
    {
        const std::string &param = message.String();
        const auto commIdx = FindCommand(param.c_str());
        if (commIdx >= 0)
            CommandExecute(pCommandsList[commIdx].code);
    }
    break;

    case 4: // Change position
    {
        XYRECT rect;
        rect.left = message.Long();
        rect.top = message.Long();
        rect.right = message.Long();
        rect.bottom = message.Long();
        ChangePosition(rect);
    }
    break;
    }
    return 0;
}

bool CINODE::CheckCommandUsed(int comCode) const
{
    int i;
    for (i = 0; i < COMMAND_QUANTITY; i++)
        if (pCommandsList[i].code == comCode)
            break;
    if (i == COMMAND_QUANTITY)
        return false;
    return m_pCommands[i].bUse;
}

bool CINODE::Init(const Config& node_config, const Config& def_config,
        VDX9RENDER *rs, XYRECT &hostRect, XYPOINT &ScreenSize) {
    if (!rs) {
        return false;
    }

    std::pair<const Config&, const Config&> configs{node_config, def_config};
    m_rs = rs;
    m_screenSize = ScreenSize;
    m_hostRect = hostRect;

    auto pos_opt = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "position");
    if (pos_opt.has_value()) {
        m_rect = *pos_opt;
    } else {
        m_rect = m_hostRect;
    }
    m_nAbsoluteRectVal = static_cast<int>(Config::GetOrGet<std::int64_t>(configs, "bAbsoluteRectangle", 0));

    GetAbsoluteRect(m_rect, m_nAbsoluteRectVal);

    // glow cursor
    const auto show_clow_cursor = Config::GetOrGet<std::int64_t>(configs, "bShowGlowCursor", 1);
    SetGlowCursor(show_clow_cursor);
    const auto show_glow_cursor = Config::GetOrGet<std::int64_t>(configs, "bGlowCursorToBack", 0);
    SetGlowCursorToBack(show_glow_cursor);

    // start using
    m_bUse = !Config::GetOrGet<std::int64_t>(configs, "bNotUse", 0);

    // mouse weel reaction
    m_bMouseWeelReaction = Config::GetOrGet<std::int64_t>(configs, "bWheelUse", 0);
    m_bMouseWeelReaction = Config::GetOrGet<std::int64_t>(configs, "bUseWheel", m_bMouseWeelReaction);

    m_strHelpTextureFile = Config::GetOrGet<std::string>(configs, "HelpTextureFile", {});

    m_frectHelpTextureUV = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "position", {0, 0, 1, 1}).to<double>();

    auto glow_rect_opt = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "GlowRectangle");
    if (glow_rect_opt.has_value()) {
        m_rectUserGlowCursor.left = glow_rect_opt->x;
        m_rectUserGlowCursor.top = glow_rect_opt->y;
        m_rectUserGlowCursor.right = glow_rect_opt->z;
        m_rectUserGlowCursor.bottom = glow_rect_opt->w;
        GetAbsoluteRect(m_rectUserGlowCursor, m_nAbsoluteRectVal);
    }

    auto glow_offset_opt = Config::GetOrGet<Types::Vector2<std::int64_t>>(configs, "GlowOffset");
    if (glow_offset_opt.has_value()) {
        m_bUseUserGlowOffset = true;
        m_rectUserGlowOffset.x = glow_offset_opt->x;
        m_rectUserGlowOffset.y = glow_offset_opt->y;
    }

    auto tooltip = Config::GetOrGet<std::string>(configs, "tooltip", {});
    if (!tooltip.empty()) {
        m_pToolTip = new CXI_ToolTip(pPictureService, pStringService, m_screenSize);
        Assert(m_pToolTip);
        m_pToolTip->SetByFormatString(m_rect, std::string(def_config.Name()), tooltip.c_str());
    }

    m_bMakeActionInDeclick = Config::GetOrGet<std::int64_t>(configs, "UseActionByDeclick", 0);

    LoadIni(node_config, def_config);
    return true;
}
