#pragma once

#include "../inode.h"

// picture
class CXI_LRCHANGER : public CINODE
{
  public:
    CXI_LRCHANGER();
    ~CXI_LRCHANGER() override;

    void Draw(bool bSelected, uint32_t Delta_Time) override;
    bool Init(const Storm::Filesystem::Config& node_config, const Storm::Filesystem::Config& def_config,
      VDX9RENDER *rs, XYRECT &hostRect, XYPOINT &ScreenSize) override;
    void ReleaseAll() override;
    int CommandExecute(int wActCode) override;
    bool IsClick(int buttonID, int32_t xPos, int32_t yPos) override;

    void MouseThis(float fX, float fY) override
    {
    }

    void ChangePosition(XYRECT &rNewPos) override;
    void SaveParametersToIni() override;
    int32_t GetClickState() override;

  protected:
    void LoadIni(const Storm::Filesystem::Config& node_config, const Storm::Filesystem::Config& def_config) override;

  protected:
    std::string m_sGroupName;
    int32_t m_idTex; // texture identity

    FXYRECT m_tLRect;
    FXYRECT m_tRRect;
    FXYRECT m_posLRect;
    FXYRECT m_posRRect;

    uint32_t m_dwShadowColor;
    uint32_t m_dwFaceColor;
    int32_t m_dwLightSelCol;
    int32_t m_dwDarkSelCol;

    int32_t m_dwBlindDelay;
    int32_t m_dwCurBlindState;
    bool m_bBlindIncrement;

    FXYPOINT m_ShadowShift;
    FXYPOINT m_PressShadowShift;
    FXYPOINT m_PressShift;

    int nPressedDelay;
    int nMaxDelay;

    bool m_bLeftPress;
};
