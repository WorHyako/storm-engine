#pragma once

#include "../inode.h"

// picture
class CXI_SCROLLBAR : public CINODE
{
  public:
    CXI_SCROLLBAR(CXI_SCROLLBAR &&) = delete;
    CXI_SCROLLBAR(const CXI_SCROLLBAR &) = delete;
    CXI_SCROLLBAR();
    ~CXI_SCROLLBAR() override;

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
    XYRECT GetCursorRect() override;
    uint32_t MessageProc(int32_t msgcode, MESSAGE &message) override;

  protected:
    void LoadIni(const Storm::Filesystem::Config& node_config, const Storm::Filesystem::Config& def_config) override;
    void UpdatePosition() const;

    void WriteDataToAttribute() const;
    void ChangeValue(bool bGrowing, bool bMultiply);

  protected:
    std::string m_sGroupName;
    int32_t m_idTex; // texture identity

    bool m_bPrevSelectStatus;
    FXYRECT m_rectCenterTex;
    FXYRECT m_rectSelectCenterTex;
    int32_t m_nBarWidth;
    int32_t m_nSideWidth;

    uint32_t m_dwShadowColor;
    uint32_t m_dwFaceColor;

    float m_fXShadow;
    float m_fYShadow;
    float m_fXShadowPress;
    float m_fYShadowPress;

    float m_fXDeltaPress;
    float m_fYDeltaPress;

    int m_nPressedDelay;
    int m_nMaxDelay;
    bool m_bRightPress;
    int m_clickType;

    // picture index & vertex buffers
    int32_t m_idVBuf; // identificator of the vertex buffer
    int32_t m_idIBuf; // identificator of the index buffer
    int32_t m_nVert;  // vertex quantity
    int32_t m_nIndx;  // index quantity

    FXYRECT m_frLeftTex;
    FXYRECT m_frRightTex;

    int32_t m_nFontID;
    uint32_t m_dwFontColor;
    float m_fFontScale;
    XYPOINT m_pntFontOffset;

    bool m_bShowString;
    float m_fCurValue;
    float m_fMinValue;
    float m_fMaxValue;
    float m_fStartValue;
    float m_fStepValue;
    float m_fSpeedMultiplay;

    struct MouseClickDescr
    {
        bool bExecute;
        float fCurMouseTime;
        float fDelayMouseTime;
        float fNextClickTime;
        FXYPOINT OldMousePoint;
    } m_MouseClickParam;
};
