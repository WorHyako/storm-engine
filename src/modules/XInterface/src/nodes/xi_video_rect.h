#pragma once

#include "../inode.h"

// video
class CXI_VIDEORECT : public CINODE
{
  public:
    CXI_VIDEORECT();
    ~CXI_VIDEORECT() override;
    void Draw(bool bSelected, uint32_t Delta_Time) override;
    bool Init(const Storm::Filesystem::Config& node_config, const Storm::Filesystem::Config& def_config,
        VDX9RENDER *rs, XYRECT &hostRect, XYPOINT &ScreenSize) override;
    void ReleaseAll() override;
    int CommandExecute(int wActCode) override;

    bool IsClick(int buttonID, int32_t xPos, int32_t yPos) override
    {
        return false;
    }

    void MouseThis(float fX, float fY) override
    {
    }

    void ChangePosition(XYRECT &rNewPos) override;
    void SaveParametersToIni() override;
    uint32_t MessageProc(int32_t msgcode, MESSAGE &message) override;

  protected:
    void LoadIni(const Storm::Filesystem::Config& node_config, const Storm::Filesystem::Config& def_config) override;
    void StartVideoPlay(const char *videoFile);

    uint32_t m_dwFlags;
    uint32_t m_dwColor;
    FXYRECT m_rectTex;
    entid_t m_eiVideo;
};
