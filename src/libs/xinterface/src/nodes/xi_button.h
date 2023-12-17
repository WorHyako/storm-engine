#pragma once

#include "../inode.h"

class CXI_BUTTON final : public CINODE
{
  public:
    CXI_BUTTON(CXI_BUTTON &&) = delete;
    CXI_BUTTON(const CXI_BUTTON &) = delete;
    CXI_BUTTON();
    ~CXI_BUTTON() override;

    void Draw(bool bSelected, uint32_t Delta_Time) override;
    void ReleaseAll() override;
    int CommandExecute(int wActCode) override;
    bool IsClick(int buttonID, int32_t xPos, int32_t yPos) override;

    void MouseThis(float fX, float fY) override
    {
    }

    void ChangePosition(XYRECT &rNewPos) override;
    void SaveParametersToIni() override;

    void NotUsingTime(uint32_t Delta_Time)
    {
        nPressedDelay = 0;
    }

    void SetUsing(bool bUsing) override;
    uint32_t MessageProc(int32_t msgcode, MESSAGE &message) override;

  private:
    void LoadIni(INIFILE *ini1, const char *name1, INIFILE *ini2, const char *name2) override;

    std::string groupName_;
    int32_t m_idTex;
    CVideoTexture *m_pTex;

    FXYRECT m_tRect;

    uint32_t m_argbDisableColor;
    uint32_t m_dwShadowColor;
    uint32_t m_dwFaceColor;
    uint32_t m_dwFontColor;
    uint32_t m_dwLightColor;
    uint32_t m_dwDarkColor;
    float m_fBlindSpeed;
    float m_fCurBlind;
    bool m_bUpBlind;

    float fXShadow;
    float fYShadow;
    float fXShadowPress;
    float fYShadowPress;

    float fXDeltaPress;
    float fYDeltaPress;

    int nPressedDelay;
    int nMaxDelay;

    int m_nFontNum;
    int32_t m_idString;
    int m_dwStrOffset;
};
