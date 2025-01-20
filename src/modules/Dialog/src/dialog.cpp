#include "dialog.hpp"

#include <dialog/dialog_utils.hpp>
#include <core.h>
#include <string_compare.hpp>
#include <v_sound_service.h>

#include "Filesystem/Config/Config.hpp"
#include "Filesystem/Constants/ConfigNames.hpp"

#define CNORMAL 0xFFFFFFFF
#define UNFADE_TIME 1000

#define DIALOG_BOTTOM_LINESPACE 12
#define DIALOG_TOP_LINESPACE 12

VDX9RENDER *DIALOG::RenderService{nullptr};
FRECT DIALOG::m_frScreenData;

using namespace Storm::Filesystem;
using namespace Storm::Math;

inline void SetVerticesForSquare(XI_TEX_VERTEX *pV, FRECT uv, float left, float top, float right, float bottom)
{
    pV[0].pos.x = left;
    pV[0].pos.y = top;
    pV[0].pos.z = 1.f;
    pV[0].rhw = 0.5f;
    pV[0].color = 0xFFFFFFFF;
    pV[0].u = uv.left;
    pV[0].v = uv.top;

    pV[1].pos.x = right;
    pV[1].pos.y = top;
    pV[1].pos.z = 1.f;
    pV[1].rhw = 0.5f;
    pV[1].color = 0xFFFFFFFF;
    pV[1].u = uv.right;
    pV[1].v = uv.top;

    pV[2].pos.x = left;
    pV[2].pos.y = bottom;
    pV[2].pos.z = 1.f;
    pV[2].rhw = 0.5f;
    pV[2].color = 0xFFFFFFFF;
    pV[2].u = uv.left;
    pV[2].v = uv.bottom;

    pV[3].pos.x = right;
    pV[3].pos.y = bottom;
    pV[3].pos.z = 1.f;
    pV[3].rhw = 0.5f;
    pV[3].color = 0xFFFFFFFF;
    pV[3].u = uv.right;
    pV[3].v = uv.bottom;
}

void DIALOG::DlgTextDescribe::ChangeText(const std::string_view text)
{
    asText.clear();
    pageBreaks_.clear();
    if (text.empty())
        return;

    const auto get_string_width = [this](const std::string_view &text) {
        return RenderService->StringWidth(text, nFontID, fScale);
    };

    std::vector<int32_t> forced_page_breaks{};
    size_t current_offset = 0;
    std::string_view current_span = text;
    for (;;)
    {
        const size_t next_break = current_span.find_first_of('\\', current_offset);
        if (next_break != std::string_view::npos && next_break < current_span.size() - 1)
        {
            const auto next_char = current_span[next_break + 1];
            if (storm::ichar_traits<char>::eq(next_char, 'n'))
            {
                const auto temp_text = std::string(current_span.substr(0, next_break)) + "...";
                storm::dialog::AddToStringArrayLimitedByWidth(temp_text, nWindowWidth, asText, get_string_width);
                current_span = current_span.substr(next_break + 2);
                forced_page_breaks.push_back(asText.size());
            }
            else {
                current_offset = next_break + 1;
            }
        }
        else
        {
            storm::dialog::AddToStringArrayLimitedByWidth(current_span, nWindowWidth, asText, get_string_width);
            break;
        }
    }
    currentLine_ = 0;
    pageBreaks_ = storm::dialog::SplitIntoPages(asText.size(), nShowQuantity, forced_page_breaks);
}

void DIALOG::DlgTextDescribe::Init(VDX9RENDER *pRS, D3DVIEWPORT9 &vp)
{
    auto config = Config::Load(Constants::ConfigNames::dialog());
    std::ignore = config.SelectSection("DIALOG");

    Assert(pRS);

    offset.x = 20;
    offset.y = 0;
    nWindowWidth = vp.Width - 2 * offset.x;
    offset.x += vp.X;

    nFontID = pRS->LoadFont(config.Get<std::string>("mainfont", "DIALOG2"));

    dwColor = ARGB(255, 210, 227, 227);
    dwColor = config.Get<std::int64_t>("mainFontColor", dwColor);
    fScale = GetScrHeight(config.Get<double>("mainFontScale", 1.f));
    nLineInterval = static_cast<int32_t>(pRS->CharHeight(nFontID) * fScale);

    currentLine_ = 0;
    nShowQuantity = MAX_LINES;
    nShowQuantity = config.Get<std::int64_t>("maxtextlines", nShowQuantity);
}

int32_t DIALOG::DlgTextDescribe::GetShowHeight()
{
    int32_t n;
    for (n = 0; n < pageBreaks_.size(); n++)
        if (currentLine_ < pageBreaks_[n])
            break;
    if (n < pageBreaks_.size())
        n = pageBreaks_[n] - currentLine_;
    else
        n = asText.size() - currentLine_;
    if (n > nShowQuantity)
        n = nShowQuantity;
    return (n * nLineInterval);
}

void DIALOG::DlgTextDescribe::Show(int32_t nY)
{
    int32_t n, i, y, nEnd;

    y = nY + offset.y;
    i = 0;
    for (n = 0; n < pageBreaks_.size(); n++)
        if (pageBreaks_[n] > currentLine_)
            break;
    if (n < pageBreaks_.size())
        nEnd = pageBreaks_[n];
    else
        nEnd = asText.size();
    for (n = currentLine_; i < nShowQuantity && n < nEnd; i++, n++)
    {
        RenderService->ExtPrint(nFontID, dwColor, 0, PR_ALIGN_LEFT, true, fScale, 0, 0, offset.x, y, asText[n].c_str());
        y += nLineInterval;
    }
}

bool DIALOG::DlgTextDescribe::IsLastPage()
{
    int32_t n;
    for (n = 0; n < pageBreaks_.size(); n++)
        if (pageBreaks_[n] > currentLine_)
            break;
    if (n >= pageBreaks_.size() || pageBreaks_[n] >= asText.size())
        return true;
    return false;
}

void DIALOG::DlgTextDescribe::PrevPage()
{
    int32_t n;
    for (n = pageBreaks_.size() - 1; n >= 0; n--)
        if (pageBreaks_[n] < currentLine_)
            break;
    if (n >= 0)
        currentLine_ = pageBreaks_[n];
    else
        currentLine_ = 0;
}

void DIALOG::DlgTextDescribe::NextPage()
{
    int32_t n;
    for (n = 0; n < pageBreaks_.size(); n++)
        if (pageBreaks_[n] > currentLine_)
            break;
    if (n < pageBreaks_.size() && pageBreaks_[n] < asText.size())
        currentLine_ = pageBreaks_[n];
}

//--------------------------------------------------------------------
DIALOG::DIALOG()
{
    curSnd = 0;
    forceEmergencyClose = false;
    play = -1;
    start = true;

    RenderService = nullptr;

    m_idVBufBack = -1;
    m_idIBufBack = -1;
    m_idVBufButton = -1;
    m_idIBufButton = -1;

    m_dwButtonState = 0; // no buttons

    m_nCharNameTextFont = -1;
    m_dwCharNameTextColor = 0xFFFFFFFF;

    m_bDlgChanged = true;

    strcpy_s(charDefSnd, "\0");
}

//--------------------------------------------------------------------
DIALOG::~DIALOG()
{
    core.SetTimeScale(1.f);

    if (m_idVBufBack != -1)
        RenderService->ReleaseVertexBuffer(m_idVBufBack);
    m_idVBufBack = -1;
    if (m_idIBufBack != -1)
        RenderService->ReleaseIndexBuffer(m_idIBufBack);
    m_idIBufBack = -1;

    if (m_idVBufButton != -1)
        RenderService->ReleaseVertexBuffer(m_idVBufButton);
    m_idVBufButton = -1;
    if (m_idIBufButton == -1)
        RenderService->ReleaseIndexBuffer(m_idIBufButton);
    m_idIBufButton = -1;

    if (m_nCharNameTextFont != -1)
        RenderService->UnloadFont(m_nCharNameTextFont);
    m_nCharNameTextFont = -1;
}

void DIALOG::CreateBack()
{
    const int32_t nSquareQuantity = 9 + 3 + 1; // 9-for back, 3-for name & 1-for divider
    m_nIQntBack = 6 * nSquareQuantity;         // 6 indices in one rectangle
    m_nVQntBack = 4 * nSquareQuantity;         // 4 vertices in one rectangle

    if (m_idVBufBack == -1)
        m_idVBufBack =
            RenderService->CreateVertexBuffer(XI_TEX_FVF, m_nVQntBack * sizeof(XI_TEX_VERTEX), D3DUSAGE_WRITEONLY);
    if (m_idIBufBack == -1)
        m_idIBufBack = RenderService->CreateIndexBuffer(m_nIQntBack * sizeof(uint16_t));

    auto *const pI = static_cast<uint16_t *>(RenderService->LockIndexBuffer(m_idIBufBack));
    if (pI)
    {
        for (int32_t n = 0; n < nSquareQuantity; n++)
        {
            pI[n * 6 + 0] = static_cast<uint16_t>(n * 4 + 0);
            pI[n * 6 + 1] = static_cast<uint16_t>(n * 4 + 2);
            pI[n * 6 + 2] = static_cast<uint16_t>(n * 4 + 1);
            pI[n * 6 + 3] = static_cast<uint16_t>(n * 4 + 1);
            pI[n * 6 + 4] = static_cast<uint16_t>(n * 4 + 2);
            pI[n * 6 + 5] = static_cast<uint16_t>(n * 4 + 3);
        }
        RenderService->UnLockIndexBuffer(m_idIBufBack);
    }

    // create dimensions
    m_BackParams.m_frBorderInt.left = m_BackParams.m_frBorderExt.left + m_BackParams.frBorderRect.left;
    m_BackParams.m_frBorderInt.right = m_BackParams.m_frBorderExt.right - m_BackParams.frBorderRect.right;
    m_BackParams.m_frBorderInt.top = m_BackParams.m_frBorderExt.top + m_BackParams.frBorderRect.top;
    m_BackParams.m_frBorderInt.bottom = m_BackParams.m_frBorderExt.bottom - m_BackParams.frBorderRect.bottom;
}

void DIALOG::FillBack()
{
    if (m_idVBufBack == -1)
        return;

    auto *pV = static_cast<XI_TEX_VERTEX *>(RenderService->LockVertexBuffer(m_idVBufBack));
    // center
    SetVerticesForSquare(&pV[0], m_BackParams.m_frCenterUV, m_BackParams.m_frBorderInt.left,
                         m_BackParams.m_frBorderInt.top, m_BackParams.m_frBorderInt.right,
                         m_BackParams.m_frBorderInt.bottom);
    // top
    SetVerticesForSquare(&pV[4], m_BackParams.m_frTopUV, m_BackParams.m_frBorderInt.left,
                         m_BackParams.m_frBorderExt.top, m_BackParams.m_frBorderInt.right,
                         m_BackParams.m_frBorderInt.top);
    // bottom
    SetVerticesForSquare(&pV[8], m_BackParams.m_frBottomUV, m_BackParams.m_frBorderInt.left,
                         m_BackParams.m_frBorderInt.bottom, m_BackParams.m_frBorderInt.right,
                         m_BackParams.m_frBorderExt.bottom);
    // left
    SetVerticesForSquare(&pV[12], m_BackParams.m_frLeftUV, m_BackParams.m_frBorderExt.left,
                         m_BackParams.m_frBorderInt.top, m_BackParams.m_frBorderInt.left,
                         m_BackParams.m_frBorderInt.bottom);
    // right
    SetVerticesForSquare(&pV[16], m_BackParams.m_frRightUV, m_BackParams.m_frBorderInt.right,
                         m_BackParams.m_frBorderInt.top, m_BackParams.m_frBorderExt.right,
                         m_BackParams.m_frBorderInt.bottom);
    // left top
    SetVerticesForSquare(&pV[20], m_BackParams.m_frLeftTopUV, m_BackParams.m_frBorderExt.left,
                         m_BackParams.m_frBorderExt.top, m_BackParams.m_frBorderInt.left,
                         m_BackParams.m_frBorderInt.top);
    // right top
    SetVerticesForSquare(&pV[24], m_BackParams.m_frRightTopUV, m_BackParams.m_frBorderInt.right,
                         m_BackParams.m_frBorderExt.top, m_BackParams.m_frBorderExt.right,
                         m_BackParams.m_frBorderInt.top);
    // left bottom
    SetVerticesForSquare(&pV[28], m_BackParams.m_frLeftBottomUV, m_BackParams.m_frBorderExt.left,
                         m_BackParams.m_frBorderInt.bottom, m_BackParams.m_frBorderInt.left,
                         m_BackParams.m_frBorderExt.bottom);
    // right bottom
    SetVerticesForSquare(&pV[32], m_BackParams.m_frRightBottomUV, m_BackParams.m_frBorderInt.right,
                         m_BackParams.m_frBorderInt.bottom, m_BackParams.m_frBorderExt.right,
                         m_BackParams.m_frBorderExt.bottom);

    FRECT frTL;
    frTL.top = m_BackParams.m_frBorderExt.top + m_BackParams.fpCharacterNameOffset.y;
    frTL.bottom = frTL.top + m_BackParams.fCharacterNameRectHeight;
    // left name rectangle
    frTL.left = m_BackParams.m_frBorderExt.left + m_BackParams.fpCharacterNameOffset.x;
    frTL.right = frTL.left + m_BackParams.fCharacterNameRectLeftWidth;
    SetVerticesForSquare(&pV[36], m_BackParams.frCharacterNameRectLeftUV, frTL.left, frTL.top, frTL.right, frTL.bottom);
    // medium name rectangle
    frTL.left = frTL.right;
    frTL.right += m_BackParams.fCharacterNameRectCenterWidth;
    SetVerticesForSquare(&pV[40], m_BackParams.frCharacterNameRectCenterUV, frTL.left, frTL.top, frTL.right,
                         frTL.bottom);
    // right name rectangle
    frTL.left = frTL.right;
    frTL.right += m_BackParams.fCharacterNameRectRightWidth;
    SetVerticesForSquare(&pV[44], m_BackParams.frCharacterNameRectRightUV, frTL.left, frTL.top, frTL.right,
                         frTL.bottom);

    RenderService->UnLockVertexBuffer(m_idVBufBack);
}

void DIALOG::FillDivider()
{
    if (m_idVBufBack == -1)
        return;
    if (!m_BackParams.bShowDivider)
        return;

    auto pV = static_cast<XI_TEX_VERTEX *>(RenderService->LockVertexBuffer(m_idVBufBack));
    const auto fDividerY = static_cast<float>(textViewport.Y + m_BackParams.nDividerOffsetY);
    SetVerticesForSquare(&pV[m_nVQntBack - 4], m_BackParams.m_frDividerUV,
                         m_BackParams.m_frBorderInt.left + m_BackParams.nDividerOffsetX, fDividerY,
                         m_BackParams.m_frBorderInt.right - m_BackParams.nDividerOffsetX,
                         fDividerY + m_BackParams.nDividerHeight);
    RenderService->UnLockVertexBuffer(m_idVBufBack);
}

void DIALOG::DrawBack()
{
    RenderService->TextureSet(0, m_BackParams.m_idBackTex);
    if (m_BackParams.bShowDivider)
        RenderService->DrawBuffer(m_idVBufBack, sizeof(XI_TEX_VERTEX), m_idIBufBack, 0, m_nVQntBack, 0, m_nIQntBack / 3,
                                  "texturedialogfon");
    else
        RenderService->DrawBuffer(m_idVBufBack, sizeof(XI_TEX_VERTEX), m_idIBufBack, 0, m_nVQntBack - 4, 0,
                                  m_nIQntBack / 3 - 2, "texturedialogfon");
}

void DIALOG::CreateButtons()
{
    m_nIQntButton = 6 * 2; // 6 indices in one rectangle
    m_nVQntButton = 4 * 2; // 4 vertices per rectangle

    if (m_idVBufButton == -1)
        m_idVBufButton =
            RenderService->CreateVertexBuffer(XI_TEX_FVF, m_nVQntButton * sizeof(XI_TEX_VERTEX), D3DUSAGE_WRITEONLY);
    if (m_idIBufButton == -1)
        m_idIBufButton = RenderService->CreateIndexBuffer(m_nIQntButton * sizeof(uint16_t));

    auto *pI = static_cast<uint16_t *>(RenderService->LockIndexBuffer(m_idIBufButton));
    if (pI)
    {
        for (int32_t n = 0; n < 2; n++)
        {
            pI[n * 6 + 0] = static_cast<uint16_t>(n * 4 + 0);
            pI[n * 6 + 1] = static_cast<uint16_t>(n * 4 + 2);
            pI[n * 6 + 2] = static_cast<uint16_t>(n * 4 + 1);
            pI[n * 6 + 3] = static_cast<uint16_t>(n * 4 + 1);
            pI[n * 6 + 4] = static_cast<uint16_t>(n * 4 + 2);
            pI[n * 6 + 5] = static_cast<uint16_t>(n * 4 + 3);
        }
        RenderService->UnLockIndexBuffer(m_idIBufButton);
    }
}

void DIALOG::FillButtons()
{
    if (m_idVBufButton == -1)
        return;

    m_dwButtonState &= ~(BUTTON_STATE_UPENABLE | BUTTON_STATE_DOWNENABLE);

    if ((!m_DlgText.IsLastPage()) || linkDescribe_.CanScrollDown())
        m_dwButtonState |= BUTTON_STATE_DOWNENABLE;

    if (m_DlgText.IsLastPage() && linkDescribe_.CanScrollUp())
        m_dwButtonState |= BUTTON_STATE_UPENABLE;

    if (m_DlgText.currentLine_ > 0)
        m_dwButtonState |= BUTTON_STATE_UPENABLE;

    auto pV = static_cast<XI_TEX_VERTEX *>(RenderService->LockVertexBuffer(m_idVBufButton));
    if (m_dwButtonState & BUTTON_STATE_UPLIGHT)
    {
        SetVerticesForSquare(
            &pV[0], m_ButtonParams.frUpLightButtonUV,
            m_BackParams.m_frBorderInt.right - m_ButtonParams.fRightOffset - m_ButtonParams.fpButtonSize.x,
            m_BackParams.m_frBorderInt.top + m_ButtonParams.fTopOffset,
            m_BackParams.m_frBorderInt.right - m_ButtonParams.fRightOffset,
            m_BackParams.m_frBorderInt.top + m_ButtonParams.fTopOffset + m_ButtonParams.fpButtonSize.y);
    }
    else
    {
        SetVerticesForSquare(
            &pV[0], m_ButtonParams.frUpNormalButtonUV,
            m_BackParams.m_frBorderInt.right - m_ButtonParams.fRightOffset - m_ButtonParams.fpButtonSize.x,
            m_BackParams.m_frBorderInt.top + m_ButtonParams.fTopOffset,
            m_BackParams.m_frBorderInt.right - m_ButtonParams.fRightOffset,
            m_BackParams.m_frBorderInt.top + m_ButtonParams.fTopOffset + m_ButtonParams.fpButtonSize.y);
    }

    if (m_dwButtonState & BUTTON_STATE_DOWNLIGHT)
    {
        SetVerticesForSquare(
            &pV[4], m_ButtonParams.frDownLightButtonUV,
            m_BackParams.m_frBorderInt.right - m_ButtonParams.fRightOffset - m_ButtonParams.fpButtonSize.x,
            m_BackParams.m_frBorderInt.bottom - m_ButtonParams.fBottomOffset - m_ButtonParams.fpButtonSize.y,
            m_BackParams.m_frBorderInt.right - m_ButtonParams.fRightOffset,
            m_BackParams.m_frBorderInt.bottom - m_ButtonParams.fBottomOffset);
    }
    else
    {
        SetVerticesForSquare(
            &pV[4], m_ButtonParams.frDownNormalButtonUV,
            m_BackParams.m_frBorderInt.right - m_ButtonParams.fRightOffset - m_ButtonParams.fpButtonSize.x,
            m_BackParams.m_frBorderInt.bottom - m_ButtonParams.fBottomOffset - m_ButtonParams.fpButtonSize.y,
            m_BackParams.m_frBorderInt.right - m_ButtonParams.fRightOffset,
            m_BackParams.m_frBorderInt.bottom - m_ButtonParams.fBottomOffset);
    }
    RenderService->UnLockVertexBuffer(m_idVBufButton);
}

void DIALOG::DrawButtons()
{
    RenderService->TextureSet(0, m_ButtonParams.m_idTexture);
    if (m_dwButtonState & BUTTON_STATE_UPENABLE)
    {
        if (m_dwButtonState & BUTTON_STATE_DOWNENABLE)
            RenderService->DrawBuffer(m_idVBufButton, sizeof(XI_TEX_VERTEX), m_idIBufButton, 0, m_nVQntButton, 0, 2 * 2,
                                      "texturedialogfon");
        else
            RenderService->DrawBuffer(m_idVBufButton, sizeof(XI_TEX_VERTEX), m_idIBufButton, 0, m_nVQntButton, 0, 2 * 1,
                                      "texturedialogfon");
    }
    else if (m_dwButtonState & BUTTON_STATE_DOWNENABLE)
        RenderService->DrawBuffer(m_idVBufButton, sizeof(XI_TEX_VERTEX), m_idIBufButton, 0, m_nVQntButton, 6, 2 * 1,
                                  "texturedialogfon");
}

void DIALOG::LoadFromIni() {
    auto config = Config::Load(Constants::ConfigNames::dialog());
    std::ignore = config.SelectSection("BACKPARAM");

    const auto texture_path = config.Get<std::string>("texture", "dialog\\interface.tga");
    m_BackParams.m_idBackTex = RenderService->TextureCreate(texture_path.c_str());

    FPOINT fpScrSize{config.Get<Types::Vector2<std::int64_t>>("baseScreenSize", {-1, -1}).to<double>()};
    FPOINT fpScrOffset{config.Get<Types::Vector2<std::int64_t>>("baseScreenOffset",{}).to<double>()};
    const auto &screenSize = core.GetScreenSize();
    fpScrSize.x = fpScrSize.x <= 0.0f
        ? static_cast<float>(screenSize.width)
        : fpScrSize.x;
    fpScrSize.y = fpScrSize.y <= 0.0f
        ? static_cast<float>(screenSize.height)
        : fpScrSize.y;
    m_nScrBaseWidth = static_cast<std::int32_t>(fpScrSize.x);
    m_nScrBaseHeight = static_cast<std::int32_t>(fpScrSize.y);
    D3DVIEWPORT9 vp;
    RenderService->GetViewport(&vp);
    m_frScreenData.right = vp.Width / (fpScrSize.x + fpScrOffset.x);
    m_frScreenData.bottom = vp.Height / (fpScrSize.y + fpScrOffset.y);
    m_frScreenData.left = fpScrOffset.x / (fpScrSize.x + fpScrOffset.x);
    m_frScreenData.top = fpScrOffset.y / (fpScrSize.y + fpScrOffset.y);

    Types::Vector4<double> def_value{0.0, 0.0, 1.0, 1.0};
    m_BackParams.m_frLeftTopUV = config.Get<Types::Vector4<double>>("uvLeftTop", def_value);
    m_BackParams.m_frRightTopUV = config.Get<Types::Vector4<double>>("uvRightTop", def_value);
    m_BackParams.m_frLeftBottomUV = config.Get<Types::Vector4<double>>("uvLeftBottom", def_value);
    m_BackParams.m_frRightBottomUV = config.Get<Types::Vector4<double>>("uvRightBottom", def_value);
    m_BackParams.m_frLeftUV = config.Get<Types::Vector4<double>>("uvLeft", def_value);
    m_BackParams.m_frRightUV = config.Get<Types::Vector4<double>>("uvRight", def_value);
    m_BackParams.m_frTopUV = config.Get<Types::Vector4<double>>("uvTop", def_value);
    m_BackParams.m_frBottomUV = config.Get<Types::Vector4<double>>("uvBottom", def_value);
    m_BackParams.m_frCenterUV = config.Get<Types::Vector4<double>>("uvCenter", def_value);
    m_BackParams.m_frDividerUV = config.Get<Types::Vector4<double>>("uvDivider", def_value);

    m_BackParams.frCharacterNameRectLeftUV = config.Get<Types::Vector4<double>>("uvChrNameLeft", def_value);
    m_BackParams.frCharacterNameRectRightUV = config.Get<Types::Vector4<double>>("uvChrNameRight", def_value);
    m_BackParams.frCharacterNameRectCenterUV = config.Get<Types::Vector4<double>>("uvChrNameCenter", def_value);
    m_BackParams.fpCharacterNameOffset = config.Get<Types::Vector2<double>>("chrNameOffset", {});
    m_BackParams.fpCharacterNameOffset.x = GetScrWidth(m_BackParams.fpCharacterNameOffset.x);
    m_BackParams.fpCharacterNameOffset.y = GetScrHeight(m_BackParams.fpCharacterNameOffset.y);
    m_BackParams.fCharacterNameRectHeight = GetScrHeight(config.Get<std::int64_t>("ChrNameHeight", 32));
    m_BackParams.fCharacterNameRectLeftWidth = GetScrWidth(config.Get<std::int64_t>("ChrNameLeftWidth", 16));
    m_BackParams.fCharacterNameRectCenterWidth = GetScrWidth(config.Get<std::int64_t>("ChrNameCenterWidth", 128));
    m_BackParams.fCharacterNameRectRightWidth = GetScrWidth(config.Get<std::int64_t>("ChrNameRightWidth", 16));

    m_BackParams.bShowDivider = false;
    m_BackParams.nDividerHeight = GetScrHeight(config.Get<std::int64_t>("dividerHeight", 8));
    m_BackParams.nDividerOffsetX = GetScrWidth(config.Get<std::int64_t>("dividerOffsetX", 8));
    m_BackParams.nDividerOffsetY = 0;

    m_BackParams.frBorderRect = config.Get<Types::Vector4<std::int64_t>>("backBorderOffset", {0, 0, 1, 1}).to<double>();
    m_BackParams.frBorderRect.left = GetScrWidth(m_BackParams.frBorderRect.left);
    m_BackParams.frBorderRect.right = GetScrWidth(m_BackParams.frBorderRect.right);
    m_BackParams.frBorderRect.top = GetScrHeight(m_BackParams.frBorderRect.top);
    m_BackParams.frBorderRect.bottom = GetScrHeight(m_BackParams.frBorderRect.bottom);

    m_BackParams.m_frBorderExt = config.Get<Types::Vector4<std::int64_t>>("backPosition", {0, 0, 1, 1}).to<double>();
    m_BackParams.m_frBorderExt.left = GetScrX(m_BackParams.m_frBorderExt.left);
    m_BackParams.m_frBorderExt.right = GetScrX(m_BackParams.m_frBorderExt.right);
    m_BackParams.m_frBorderExt.top = GetScrY(m_BackParams.m_frBorderExt.top);
    m_BackParams.m_frBorderExt.bottom = GetScrY(m_BackParams.m_frBorderExt.bottom);

    m_ButtonParams.m_idTexture = m_BackParams.m_idBackTex;
    std::ignore = config.SelectSection("BUTTON");
    m_ButtonParams.frUpNormalButtonUV = config.Get<Types::Vector4<double>>("uvUpNormal", def_value);
    m_ButtonParams.frDownNormalButtonUV = config.Get<Types::Vector4<double>>("uvDownNormal", def_value);
    m_ButtonParams.frUpLightButtonUV = config.Get<Types::Vector4<double>>("uvUpLight", def_value);
    m_ButtonParams.frDownLightButtonUV = config.Get<Types::Vector4<double>>("uvDownLight", def_value);
    m_ButtonParams.fpButtonSize = config.Get<Types::Vector2<std::int64_t>>("buttonSize", {}).to<double>();
    m_ButtonParams.fRightOffset = GetScrWidth(config.Get<double>("rightoffset", 0.0));
    m_ButtonParams.fTopOffset = GetScrHeight(config.Get<double>("topoffset", 0.0));
    m_ButtonParams.fBottomOffset = GetScrHeight(config.Get<double>("bottomoffset", 0.0));

    std::ignore = config.SelectSection("DIALOG");
    m_nCharNameTextFont = RenderService->LoadFont(config.Get<std::string>("charnamefont", "DIALOG2"));
    m_dwCharNameTextColor = config.Get<std::int64_t>("charnamecolor", 0xFFFFFFFF);
    m_fCharNameTextScale = config.Get<double>("charnamescale", 1.0);

    m_fpCharNameTextOffset = config.Get<Types::Vector2<std::int64_t>>("charnameoffset", {}).to<double>();
    m_fpCharNameTextOffset.x = GetScrWidth(m_fpCharNameTextOffset.x);
    m_fpCharNameTextOffset.y = GetScrHeight(m_fpCharNameTextOffset.y);
}

//--------------------------------------------------------------------
bool DIALOG::Init()
{
    forceEmergencyClose = false;
    selectedLinkName[0] = 0;
    core.SetTimeScale(0.f);
    unfadeTime = 0;

    RenderService = static_cast<VDX9RENDER *>(core.GetService("dx9render"));
    Assert(RenderService);

    snd = static_cast<VSoundService *>(core.GetService("SoundService"));
    // Assert( snd );

    //----------------------------------------------------------
    LoadFromIni();

    CreateBack();
    FillBack();
    FillDivider();

    textViewport.X = static_cast<int32_t>(m_BackParams.m_frBorderInt.left + GetScrWidth(4));
    textViewport.Y = static_cast<int32_t>(GetScrY(437));
    textViewport.Width = static_cast<int32_t>(m_BackParams.m_frBorderInt.right - GetScrWidth(4)) - textViewport.X;
    textViewport.Height = static_cast<uint32_t>(GetScrHeight(66));
    textViewport.MinZ = 0.0f;
    textViewport.MaxZ = 1.0f;

    m_DlgText.Init(RenderService, textViewport);
    InitLinks(RenderService, textViewport);

    CreateButtons();
    FillButtons();

    return true;
}

void DIALOG::InitLinks(VDX9RENDER *pRS, D3DVIEWPORT9 &vp)
{
    linkDescribe_.Init();

    linkDescribe_.SetAttributes(AttributesPointer);
    linkDescribe_.SetRenderer(pRS);

    POINT offset{20, 0};
    offset.x += vp.X;
    linkDescribe_.SetOffset(offset);

    int32_t window_width = vp.Width - 2 * offset.x;
    linkDescribe_.SetWindowWidth(window_width);

    auto config = Config::Load(Constants::ConfigNames::dialog());
    std::ignore = config.SelectSection("DIALOG");

    int32_t font_id = pRS->LoadFont(config.Get<std::string>("subfont", "DIALOG3"));
    linkDescribe_.SetFont(font_id);

    float scale = GetScrHeight(config.Get<double>("subFontScale", 1.f));
    linkDescribe_.SetFontScale(scale);

    int32_t line_height = static_cast<int32_t>(pRS->CharHeight(font_id) * scale * .9f);
    linkDescribe_.SetLineHeight(line_height);

    linkDescribe_.SetMaxLinesPerPage(config.Get<std::int64_t>("maxlinkslines", 5));

    linkDescribe_.SetColor(config.Get<std::int64_t>("subFontColor", 0xFF808080));
    linkDescribe_.SetSelectedColor(config.Get<std::int64_t>("subFontColorSelect", 0xFFFFFFFF));
}

//--------------------------------------------------------------------
void DIALOG::Realize(uint32_t Delta_Time)
{
    RenderService->MakePostProcess();
    // delayed exit from pause
    if (unfadeTime <= UNFADE_TIME)
    {
        unfadeTime += static_cast<int>(core.GetRDeltaTime());
        float timeK = static_cast<float>(unfadeTime) / UNFADE_TIME;
        if (timeK > 1.f)
            timeK = 1.f;
        core.SetTimeScale(timeK);
    }

    // play speech
    if (play == 0 && soundName[0] && snd)
    {
        curSnd = snd->SoundPlay(soundName, PCM_STEREO, VOLUME_SPEECH);
        play = 1;
    }

    // no dialogue attributes - no further conversation
    if (!AttributesPointer)
        return;

    if (m_bDlgChanged)
    {
        UpdateDlgTexts();
        UpdateDlgViewport();
        FillDivider();
        FillButtons();
    }

    CONTROL_STATE cs;
    CONTROL_STATE cs1; // boal
    CONTROL_STATE cs2;

    // interruption of dialogue
    core.Controls->GetControlState("DlgCancel", cs);
    if (cs.state == CST_ACTIVATED)
        core.Event("DialogCancel");

    bool bDoUp = false;
    bool bDoDown = false;
    //
    core.Controls->GetControlState("DlgUp", cs);
    if (cs.state == CST_ACTIVATED)
        bDoUp = true;
    if (!linkDescribe_.IsInEditMode())
    {
        core.Controls->GetControlState("DlgUp2", cs);
        if (cs.state == CST_ACTIVATED)
            bDoUp = true;
        core.Controls->GetControlState("DlgUp3", cs);
        if (cs.state == CST_ACTIVATED)
            bDoUp = true;
    }
    //
    core.Controls->GetControlState("DlgDown", cs);
    if (cs.state == CST_ACTIVATED)
        bDoDown = true;
    if (!linkDescribe_.IsInEditMode())
    {
        core.Controls->GetControlState("DlgDown2", cs);
        if (cs.state == CST_ACTIVATED)
            bDoDown = true;
        core.Controls->GetControlState("DlgDown3", cs);
        if (cs.state == CST_ACTIVATED)
            bDoDown = true;
    }

    // go up (to the previous line)
    if (bDoUp)
    {
        // play click of the pressed key
        if (snd)
            snd->SoundPlay(DEFAULT_TICK_SOUND, PCM_STEREO, VOLUME_FX);

        if (m_DlgText.IsLastPage())
        {
            if (linkDescribe_.CanMoveUp())
            {
                linkDescribe_.MoveUp();
                FillButtons();
            }
        }
    }
    // go down - to the next line
    if (bDoDown)
    {
        // play click of the pressed key
        if (snd)
            snd->SoundPlay(DEFAULT_TICK_SOUND, PCM_STEREO, VOLUME_FX);

        if (m_DlgText.IsLastPage())
        {
            if (linkDescribe_.CanMoveDown())
            {
                linkDescribe_.MoveDown();
                FillButtons();
            }
        }
    }
    // page up
    core.Controls->GetControlState("DlgScrollUp", cs);
    if (cs.state == CST_ACTIVATED)
    {
        if (snd)
            snd->SoundPlay(DEFAULT_TICK_SOUND, PCM_STEREO, VOLUME_FX);
        if (m_DlgText.currentLine_ > 0)
        {
            m_DlgText.PrevPage();
            UpdateDlgViewport();
            FillDivider();
            FillButtons();
        }
    }
    // page down
    core.Controls->GetControlState("DlgScrollDown", cs);
    if (cs.state == CST_ACTIVATED)
    {
        if (snd)
            snd->SoundPlay(DEFAULT_TICK_SOUND, PCM_STEREO, VOLUME_FX);
        if (!m_DlgText.IsLastPage())
        {
            m_DlgText.NextPage();
            UpdateDlgViewport();
            FillDivider();
            FillButtons();
        }
    }
    // action
    core.Controls->GetControlState("DlgAction", cs);
    core.Controls->GetControlState("DlgAction2", cs2);
    core.Controls->GetControlState("DlgAction1", cs1); // boal
    if (linkDescribe_.IsInEditMode())
    {
        cs.state = CST_INACTIVE;
    }
    if (cs.state == CST_ACTIVATED || cs2.state == CST_ACTIVATED || cs1.state == CST_ACTIVATED) // boal
    {
        // play click of the pressed key
        if (snd)
            snd->SoundPlay(DEFAULT_TICK_SOUND, PCM_STEREO, VOLUME_FX);

        if (m_DlgText.IsLastPage())
        {
            // showing answer options
            ATTRIBUTES *pA = AttributesPointer->GetAttributeClass("links");
            if (pA)
                pA = pA->GetAttributeClass(linkDescribe_.GetSelectedLine());
            if (pA)
            {
                const char *goName = pA->GetAttribute("go");
                if (!goName || storm::iEquals(goName, selectedLinkName))
                    EmergencyExit();
                else
                {
                    AttributesPointer->SetAttribute("CurrentNode", goName);
                    strcpy_s(selectedLinkName, goName);

                    // set default
                    strcpy_s(soundName, charDefSnd);
                    core.Event("DialogEvent");
                }
            }
        }
        else
        {
            // still showing the text
            m_DlgText.NextPage();
            UpdateDlgViewport();
            FillDivider();
            FillButtons();
        }
    }

    DrawBack();
    DrawButtons();

    RenderService->ExtPrint(m_nCharNameTextFont, m_dwCharNameTextColor, 0, PR_ALIGN_LEFT, true, m_fCharNameTextScale, 0,
                            0, static_cast<int32_t>(m_BackParams.m_frBorderExt.left + m_fpCharNameTextOffset.x),
                            static_cast<int32_t>(m_BackParams.m_frBorderExt.top + m_fpCharNameTextOffset.y), "%s",
                            m_sTalkPersName.c_str());

    m_DlgText.Show(textViewport.Y);
    if (m_DlgText.IsLastPage())
        linkDescribe_.Show(
            static_cast<int32_t>(textViewport.Y + m_BackParams.nDividerOffsetY + m_BackParams.nDividerHeight));

    if (snd && !snd->SoundIsPlaying(curSnd))
    {
        // stop animation
        if (play == 1) // if person speech
            play = -1;
    }
}

//--------------------------------------------------------------------
uint32_t DIALOG::AttributeChanged(ATTRIBUTES *pA)
{
    // search for default settings
    bool parLinks = false;
    ATTRIBUTES *par = pA->GetParent();
    if (par != nullptr)
    {
        const char *parname = par->GetThisName();
        if (parname != nullptr && storm::iEquals(parname, "Links"))
            parLinks = true;
    }

    const char *nm = pA->GetThisName();

    // play sound d.speech
    if (!parLinks && nm && storm::iEquals(nm, "greeting")) // was "snd"
    {
        strcpy_s(soundName, pA->GetThisAttr());
        if (start)
            play = 0;
    }

    m_bDlgChanged = true;
    return 0;
}

//--------------------------------------------------------------------
uint64_t DIALOG::ProcessMessage(MESSAGE &message)
{
    switch (message.Long())
    {
        // get character ID
    case 0:
        persId = message.EntityID();
        persMdl = message.EntityID();
        break;
        // get person ID
    case 1:
        charId = message.EntityID();
        charMdl = message.EntityID();
        const char *attr = nullptr;
        if (attr = core.Entity_GetAttribute(charId, "name"))
        {
            m_sTalkPersName = attr;
        }
        if (attr = core.Entity_GetAttribute(charId, "lastname"))
        {
            if (m_sTalkPersName.size() > 0 && *attr != '\0')
            {
                m_sTalkPersName += " ";
            }
            m_sTalkPersName += attr;
        }
        m_BackParams.fCharacterNameRectCenterWidth =
            4.f + RenderService->StringWidth(m_sTalkPersName, m_nCharNameTextFont, m_fCharNameTextScale);
        break;
    }
    return 0;
}

//--------------------------------------------------------------------
void DIALOG::EmergencyExit()
{
    if (forceEmergencyClose)
        return;
    forceEmergencyClose = true;
    core.Trace("DIALOG: Invalid links, emergency exit! (last link = %s)", selectedLinkName);
    core.Event("EmergencyDialogExit");
}

void DIALOG::UpdateDlgTexts()
{
    if (!AttributesPointer)
        return;

    const std::string &text = AttributesPointer->GetAttribute("Text");
    m_DlgText.ChangeText(text);
    linkDescribe_.ChangeText(AttributesPointer->GetAttributeClass("Links"));

    m_bDlgChanged = false;
}

void DIALOG::UpdateDlgViewport()
{
    const int32_t nTextHeight = m_DlgText.GetShowHeight();
    const int32_t nLinksHeight = m_DlgText.IsLastPage() ? linkDescribe_.GetShowHeight() : 0;

    int32_t nAllHeight = nTextHeight;
    if (nLinksHeight > 0)
    {
        m_BackParams.bShowDivider = true;
        m_BackParams.nDividerOffsetY = nTextHeight;
        nAllHeight += nLinksHeight + static_cast<int32_t>(m_BackParams.nDividerHeight);
    }
    else
    {
        m_BackParams.bShowDivider = false;
    }

    textViewport.Height = nAllHeight;
    textViewport.Y = static_cast<uint32_t>(static_cast<int32_t>(m_BackParams.m_frBorderInt.bottom) - nAllHeight -
                                           GetScrHeight(DIALOG_BOTTOM_LINESPACE));

    const float fTopBorder = textViewport.Y - GetScrHeight(DIALOG_TOP_LINESPACE);
    if (m_BackParams.m_frBorderInt.top != fTopBorder)
    {
        m_BackParams.m_frBorderInt.top = fTopBorder;
        m_BackParams.m_frBorderExt.top = fTopBorder - m_BackParams.frBorderRect.top;

        FillBack();
    }
}
