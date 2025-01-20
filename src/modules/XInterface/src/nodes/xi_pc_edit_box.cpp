#include "xi_pc_edit_box.h"

#include "string_compare.hpp"
#include "xi_image.h"
#include "xi_util.h"
#include <stdio.h>

#include <string>

#define WIDTH_SCALE_USED 0.9f
#define HEIGHT_SCALE_USED 0.9f

using namespace Storm::Filesystem;
using namespace Storm::Math;

CXI_PCEDITBOX::CXI_PCEDITBOX()
    : m_nStringAlign(0), m_nMaxSize(0), m_nMaxWidth(0)
{
    m_nFontID = -1;
    m_dwFontColor = 0xFFFFFFFF;
    m_pntFontOffset.x = 0;
    m_pntFontOffset.y = 0;
    m_fFontScale = 0;

    m_pLeftImage = nullptr;
    m_pRightImage = nullptr;
    m_pMiddleImage = nullptr;

    m_nEditPos = -1;
    m_nFirstShowCharacterIndex = 0;
    m_bWaitKeyRelease = true;
    m_bDisguiseString = false;
}

CXI_PCEDITBOX::~CXI_PCEDITBOX()
{
    ReleaseAll();
}

void CXI_PCEDITBOX::ReleaseAll()
{
    FONT_RELEASE(m_rs, m_nFontID);
    STORM_DELETE(m_pLeftImage);
    STORM_DELETE(m_pRightImage);
    STORM_DELETE(m_pMiddleImage);
}

void CXI_PCEDITBOX::Draw(bool bSelected, uint32_t Delta_Time)
{
    if (m_pLeftImage)
        m_pLeftImage->Draw();
    if (m_pRightImage)
        m_pRightImage->Draw();
    if (m_pMiddleImage)
        m_pMiddleImage->Draw();

    auto x = m_rect.left + m_pntFontOffset.x;
    if (m_nStringAlign == PR_ALIGN_CENTER)
    {
        x = (m_rect.left + m_rect.right) / 2;
    }
    else if (m_nStringAlign == PR_ALIGN_RIGHT)
    {
        x = m_rect.right - m_pntFontOffset.x;
    }

    // show out string
    std::string sString;
    UpdateString(sString);
    if (!sString.empty())
    // m_rs->ExtPrint(
    // m_nFontID,m_dwFontColor,0,m_nStringAlign,true,m_fFontScale,m_screenSize.x,m_screenSize.y,m_rect.left+m_pntFontOffset.x,m_rect.top+m_pntFontOffset.y,"%s",sString.c_str()+m_nFirstShowCharacterIndex);
    {
        int offset = utf8::u8_offset(sString.c_str(), m_nFirstShowCharacterIndex);
        CXI_UTILS::PrintTextIntoWindow(m_rs, m_nFontID, m_dwFontColor, m_nStringAlign, true, m_fFontScale,
                                       m_screenSize.x, m_screenSize.y, x, m_rect.top + m_pntFontOffset.y,
                                       sString.c_str() + offset, m_rect.left, m_rect.top, m_rect.right - m_rect.left,
                                       m_rect.bottom - m_rect.top);
    }

    // show cursor position
    if (IsEditMode())
        ShowCursorPosition(sString);

    CONTROL_STATE cs;
    core.Controls->GetControlState("IStartButton", cs);
    if (cs.state == CST_INACTIVATED)
        core.Event("editexit", "s", m_nodeName);
}

bool CXI_PCEDITBOX::Init(const Config& node_config, const Config& def_config,
    VDX9RENDER *rs, XYRECT &hostRect, XYPOINT &ScreenSize) {
    if (!CINODE::Init(node_config, def_config, rs, hostRect, ScreenSize))
        return false;
    SetGlowCursor(false);
    return true;
}

int CXI_PCEDITBOX::CommandExecute(int wActCode)
{
    return -1;
}

bool CXI_PCEDITBOX::IsClick(int buttonID, int32_t xPos, int32_t yPos)
{
    if (xPos < m_rect.left)
        return false;
    if (xPos > m_rect.right)
        return false;
    if (yPos < m_rect.top)
        return false;
    if (yPos > m_rect.bottom)
        return false;
    return true;
}

void CXI_PCEDITBOX::ChangePosition(XYRECT &rNewPos)
{
    m_rect = rNewPos;

    // m_pntFontOffset.x += m_rect.left;
    // m_pntFontOffset.y += m_rect.top;

    // update position
    auto nMiddleLeft = m_rect.left;
    auto nMiddleRight = m_rect.right;
    const auto nHeight = m_rect.bottom - m_rect.top;
    if (m_pLeftImage)
    {
        if (m_pLeftImage->IsImagePresent())
        {
            m_pLeftImage->SetSize(m_pLeftImage->GetWidth(), nHeight);
            m_pLeftImage->SetPosition(m_rect.left, m_rect.top, IPType_LeftTop);
            nMiddleLeft += m_pLeftImage->GetWidth();
        }
    }
    if (m_pRightImage)
    {
        if (m_pRightImage->IsImagePresent())
        {
            m_pRightImage->SetSize(m_pRightImage->GetWidth(), nHeight);
            m_pRightImage->SetPosition(m_rect.right, m_rect.top, IPType_RightTop);
            nMiddleRight -= m_pRightImage->GetWidth();
        }
    }
    if (m_pMiddleImage)
    {
        if (m_pMiddleImage->IsImagePresent())
        {
            m_pMiddleImage->SetSize(nMiddleRight - nMiddleLeft, nHeight);
            m_pMiddleImage->SetPosition(nMiddleLeft, m_rect.top, IPType_LeftTop);
        }
    }
}

void CXI_PCEDITBOX::SaveParametersToIni()
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

void CXI_PCEDITBOX::LoadIni(const Config& node_config, const Config& def_config) {
    std::pair<const Config&, const Config&> configs{node_config, def_config};

    // get font number
    auto font = Config::GetOrGet<std::string>(configs, "strFont", {});
    if (!font.empty()) {
        m_nFontID = m_rs->LoadFont(font.c_str());
        if (m_nFontID == -1)
            core.Trace("can`t load font:'%s'", font);
    }
    // Get font scale
    m_fFontScale = Config::GetOrGet<double>(configs, "fontScale", 1.0);

    m_bDisguiseString = Config::GetOrGet<std::int64_t>(configs, "disguisestring", 0);

    // Get font color
    auto font_color = Config::GetOrGet<Types::Vector4<std::int64_t>>(configs, "argbFontColor", {});
    m_dwFontColor = ARGB(font_color.x, font_color.y, font_color.z, font_color.w);

    // Get max string size for edit string
    m_nMaxSize = Config::GetOrGet<std::int64_t>(configs, "stringLength", -1);
    m_nMaxWidth = Config::GetOrGet<std::int64_t>(configs, "stringWidth", -1);

    m_pntFontOffset = Config::GetOrGet<Types::Vector2<std::int64_t>>(configs, "stringoffset", {m_pntFontOffset.x, m_pntFontOffset.y});
    m_nStringAlign = PR_ALIGN_LEFT;
    auto string_align = Config::GetOrGet<std::string>(configs, "stringalign", "center");
    if (string_align == "center")
        m_nStringAlign = PR_ALIGN_CENTER;
    else if (string_align == "right")
        m_nStringAlign = PR_ALIGN_RIGHT;

    // m_pntFontOffset.x += m_rect.left;
    // m_pntFontOffset.y += m_rect.top;

    // read images
    auto left_image = Config::GetOrGet<std::string>(configs, "leftImage", {});
    if (!left_image.empty()) {
        m_pLeftImage = new CXI_IMAGE;
        if (m_pLeftImage)
            m_pLeftImage->LoadAccordingToString(left_image.c_str());
    }
    auto right_image = Config::GetOrGet<std::string>(configs, "RightImage", {});
    if (!right_image.empty()) {
        m_pRightImage = new CXI_IMAGE;
        if (m_pRightImage)
            m_pRightImage->LoadAccordingToString(right_image.c_str());
    }
    auto middle_image_vec = Config::GetOrGet<std::vector<std::string>>(configs, "MiddleImage", {});
    std::stringstream ss;
    std::ranges::for_each(middle_image_vec,
        [&ss](const auto& each) {
            ss << each << ',';
    });
    std::string middle_image{ss.str()};
    middle_image.erase(std::size(middle_image));
    if (!middle_image.empty()) {
        m_pMiddleImage = new CXI_IMAGE;
        if (m_pMiddleImage)
            m_pMiddleImage->LoadAccordingToString(middle_image.c_str());
    }

    auto excluce_chars_vec = Config::GetOrGet<std::vector<std::string>>(configs, "excludechars", {});
    ss.clear();
    std::ranges::for_each(excluce_chars_vec,
        [&ss](const auto& each) {
        ss << each << ',';
    });
    m_sExcludeChars = ss.str();
    // update position
    auto nMiddleLeft = m_rect.left;
    auto nMiddleRight = m_rect.right;
    const auto nHeight = m_rect.bottom - m_rect.top;
    if (m_pLeftImage) {
        if (m_pLeftImage->IsImagePresent()) {
            m_pLeftImage->SetSize(m_pLeftImage->GetWidth(), nHeight);
            m_pLeftImage->SetPosition(m_rect.left, m_rect.top, IPType_LeftTop);
            nMiddleLeft += m_pLeftImage->GetWidth();
        } else {
            STORM_DELETE(m_pLeftImage);
        }
    }
    if (m_pRightImage) {
        if (m_pRightImage->IsImagePresent()) {
            m_pRightImage->SetSize(m_pRightImage->GetWidth(), nHeight);
            m_pRightImage->SetPosition(m_rect.right, m_rect.top, IPType_RightTop);
            nMiddleRight -= m_pRightImage->GetWidth();
        } else {
            STORM_DELETE(m_pRightImage);
        }
    }
    if (nMiddleLeft >= nMiddleRight) {
        STORM_DELETE(m_pMiddleImage);
    }
    if (m_pMiddleImage) {
        if (m_pMiddleImage->IsImagePresent()) {
            m_pMiddleImage->SetSize(nMiddleRight - nMiddleLeft, nHeight);
            m_pMiddleImage->SetPosition(nMiddleLeft, m_rect.top, IPType_LeftTop);
        } else {
            STORM_DELETE(m_pMiddleImage);
        }
    }

    auto *pA = ptrOwner->AttributesPointer->GetAttributeClass(m_nodeName);
    if (!pA)
        pA = ptrOwner->AttributesPointer->CreateSubAClass(ptrOwner->AttributesPointer, m_nodeName);
    if (pA && !pA->GetAttribute("str"))
        pA->CreateAttribute("str", "");
}

void CXI_PCEDITBOX::UpdateString(std::string &str)
{
    str = "";
    m_nFirstShowCharacterIndex = 0;

    ATTRIBUTES *pA = core.Entity_GetAttributeClass(g_idInterface, m_nodeName);
    if (!pA)
    {
        core.Entity_SetAttribute(g_idInterface, m_nodeName, "");
        pA = core.Entity_GetAttributeClass(g_idInterface, m_nodeName);
    }
    if (!pA)
        return;
    str = to_string(pA->GetAttribute("str"));
    int strLength = utf8::Utf8StringLength(str.c_str());
    if (m_nEditPos < 0)
        m_nEditPos = strLength;

    if (IsEditMode())
    {
        if (m_bWaitKeyRelease && core.Controls->GetKeyBufferLength() == 0)
            m_bWaitKeyRelease = false;

        if (!m_bWaitKeyRelease)
        {
            const KeyDescr *pKeys = core.Controls->GetKeyBuffer();
            for (int32_t n = 0; n < core.Controls->GetKeyBufferLength(); n++)
            {
                if (pKeys[n].bSystem)
                {
                    switch (pKeys[n].ucVKey.c)
                    {
                        // control symbols
                    case VK_BACK:
                        if (m_nEditPos > 0)
                        {
                            m_nEditPos--;
                            int offset = utf8::u8_offset(str.c_str(), m_nEditPos);
                            str[offset] = 0;
                        }
                        break;
                    case VK_END:
                        m_nEditPos = strLength;
                        break;
                    case VK_HOME:
                        m_nEditPos = 0;
                        break;
                    case VK_DELETE:
                        if (m_nEditPos < strLength)
                        {
                            int offset = utf8::u8_offset(str.c_str(), m_nEditPos);
                            int length = utf8::u8_inc(str.c_str() + offset);
                            str.erase(offset, length);
                        }
                        break;
                    case VK_LEFT:
                        if (m_nEditPos > 0)
                            m_nEditPos--;
                        break;
                    case VK_RIGHT:
                        if (m_nEditPos < strLength)
                            m_nEditPos++;
                        break;
                    }
                }
                else
                    InsertSymbol(str, pKeys[n].ucVKey);
            }
            if (pA)
                pA->SetAttribute("str", (char *)str.c_str());
            /*char chr = GetInputSymbol();
            if( chr )
            {
              switch( chr )
              {
              // control characters
              case SpecSymbol_back:
                if( m_nEditPos>0 ) {
                  m_nEditPos--;
                  str.STORM_DELETE(m_nEditPos,1);
                }
                break;
              case SpecSymbol_end:    m_nEditPos = str.size(); break;
              case SpecSymbol_home:    m_nEditPos = 0; break;
              case SpecSymbol_delete:
                if( m_nEditPos < (int32_t)str.size() )
                  str.STORM_DELETE( m_nEditPos, 1 );
                break;
              case SpecSymbol_left: if( m_nEditPos > 0 ) m_nEditPos--; break;
              case SpecSymbol_right: if( m_nEditPos < (int32_t)str.size() ) m_nEditPos++; break;

              // skip unnecessary characters
              case SpecSymbol_up:
              case SpecSymbol_down:
              case SpecSymbol_tab:
              case SpecSymbol_return:
              case SpecSymbol_escape:
                break;

              // and this is what we enter
              default:
                InsertSymbol( str, chr );
              }
              if( pA ) pA->SetAttribute( "str", (char*)str.c_str() );
            }*/

            if (m_bDisguiseString)
                DisguiseString(str);

            // defining the first character to display
            char param[2048];
            param[sizeof(param) - 1] = 0;
            sprintf_s(param, sizeof(param) - 1, "%s", str.c_str());
            for (m_nFirstShowCharacterIndex = 0; m_nFirstShowCharacterIndex < m_nEditPos; m_nFirstShowCharacterIndex++)
            {
                int offset = utf8::u8_offset(param, m_nFirstShowCharacterIndex);
                if (m_rs->StringWidth(param + offset, m_nFontID, m_fFontScale) <=
                    (m_rect.right - m_rect.left - 2 * m_pntFontOffset.x))
                    break;
            }
        }
        else
        {
            if (m_bDisguiseString)
                DisguiseString(str);
        }
    }
    else
    {
        m_bWaitKeyRelease = true;
        if (m_bDisguiseString)
            DisguiseString(str);
    }
}

void CXI_PCEDITBOX::ShowCursorPosition(std::string &str)
{
    if (m_nEditPos < 0)
        m_nEditPos = 0;

    int strLength = utf8::Utf8StringLength(str.c_str());
    if (m_nEditPos > strLength)
        m_nEditPos = strLength;

    std::string strForPosCalculate = str;
    int editOffset = utf8::u8_offset(str.c_str(), m_nEditPos);
    strForPosCalculate.erase(editOffset, str.size());
    if (m_nFirstShowCharacterIndex < 0 || m_nFirstShowCharacterIndex > m_nEditPos)
        return;

    int offset = utf8::u8_offset(str.c_str(), m_nFirstShowCharacterIndex);
    int32_t nPos = m_rs->StringWidth(strForPosCalculate.c_str() + offset, m_nFontID, m_fFontScale);

    if (m_nStringAlign == PR_ALIGN_CENTER)
        nPos -= m_rs->StringWidth((char *)str.c_str() + offset, m_nFontID, m_fFontScale) / 2;
    else if (m_nStringAlign == PR_ALIGN_RIGHT)
        nPos -= m_rs->StringWidth((char *)str.c_str() + offset, m_nFontID, m_fFontScale);

    int32_t x = m_rect.left + m_pntFontOffset.x;
    if (m_nStringAlign == PR_ALIGN_CENTER)
    {
        x = (m_rect.left + m_rect.right) / 2;
    }
    else if (m_nStringAlign == PR_ALIGN_RIGHT)
    {
        x = m_rect.right - m_pntFontOffset.x;
    }

    m_rs->ExtPrint(m_nFontID, m_dwFontColor, 0, PR_ALIGN_LEFT, true, m_fFontScale, m_screenSize.x, m_screenSize.y,
                   x + nPos, m_rect.top + m_pntFontOffset.y, "_");
}

void CXI_PCEDITBOX::InsertSymbol(std::string &str, utf8::u8_char chr)
{
    if (chr.c < 32)
        return;
    if (IsExcludeChar(chr))
        return;

    std::string chrInsert(chr.b, chr.l);
    int strLength = utf8::Utf8StringLength(str.c_str());
    // check for maximum number of characters
    if (m_nMaxSize >= 0 && strLength >= m_nMaxSize)
        return;

    int offset = utf8::u8_offset(str.c_str(), m_nEditPos);
    // checking for maximum line width
    if (m_nMaxWidth >= 0)
    {
        std::string strResult = str;
        strResult.insert(offset, chrInsert.c_str());
        if (m_rs->StringWidth(strResult.c_str(), m_nFontID, m_fFontScale) > m_nMaxWidth)
            return;
    }
    str.insert(offset, chrInsert.c_str());
    m_nEditPos++;
    strLength++; // account for the added char
    if (m_nEditPos > strLength)
        m_nEditPos = strLength;
}

void CXI_PCEDITBOX::DisguiseString(std::string &str)
{
    const int32_t q = str.size();
    for (int32_t n = 0; n < q; n++)
        str[n] = '*';
}

bool CXI_PCEDITBOX::IsExcludeChar(utf8::u8_char chr) const
{
    std::string fndStr(chr.b, chr.l);

    // if( m_sExcludeChars.FindSubStr(fndStr)>=0 ) return true;
    return m_sExcludeChars.find(fndStr) != std::string::npos;
}
