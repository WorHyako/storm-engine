#pragma once

#include "../inode.h"

// static inactive images into one object
class CXI_STATUSLINE : public CINODE
{
  public:
    CXI_STATUSLINE();
    ~CXI_STATUSLINE() override;

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
    uint32_t MessageProc(int32_t msgcode, MESSAGE &message) override;

  protected:
    void LoadIni(const Storm::Filesystem::Config& node_config, const Storm::Filesystem::Config& def_config) override;
    void Refresh() const;

  protected:
    std::string m_sGroupName;  // image list name
    int32_t m_idTex;        // texture identificator
    int32_t m_vBuf;         // vertex buffer identificator
    int32_t m_iBuf;         // index buffer identificator
    int32_t m_nVert;        // vertex quantity
    int32_t m_nIndx;        // index quantity
    float m_fLineOffset; // edge offset for frame
    FXYRECT m_texRect1;
    FXYRECT m_texRect2;
};
