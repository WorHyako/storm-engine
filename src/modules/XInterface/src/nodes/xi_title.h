#pragma once

#include "../inode.h"

class CXI_TITLE : public CINODE
{
  public:
    CXI_TITLE(CXI_TITLE &&) = delete;
    CXI_TITLE(const CXI_TITLE &) = delete;
    CXI_TITLE();
    ~CXI_TITLE() override;
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

  protected:
    void LoadIni(const Storm::Filesystem::Config& node_config, const Storm::Filesystem::Config& def_config) override;
    void FillVertexBuffer() const;

  protected:
    std::string m_sGroupName;
    int32_t m_idTex; // texture identity

    int32_t m_idString;
    XYPOINT m_StringCenter;
    uint32_t m_fontColor;
    uint32_t m_backColor;
    float m_fontScale;
    int32_t m_fontID;

    int32_t m_idVBuf; // vertex buffer identificator
    int32_t m_idIBuf; // index buffer identificator
    int32_t m_nVert;  // vertex quantity
    int32_t m_nIndx;  // index quantity

    int32_t m_nStringWidth;
    XYRECT m_mRect, m_tRect;
    int32_t m_nTiledQuantity;
};
