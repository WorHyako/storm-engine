#pragma once

#include "../inode.h"

class INIFILE;

class CXI_KEYCHANGER : public CINODE
{
  public:
    CXI_KEYCHANGER();
    ~CXI_KEYCHANGER() override;

    void Draw(bool bSelected, uint32_t Delta_Time) override;
    bool Init(const Storm::Filesystem::Config& node_config, const Storm::Filesystem::Config& def_config,
        VDX9RENDER *rs, XYRECT &hostRect, XYPOINT &ScreenSize) override;
    void ReleaseAll() override;

    int CommandExecute(int wActCode) override
    {
        return -1;
    }

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

    void LoadIni(const Storm::Filesystem::Config& node_config, const Storm::Filesystem::Config& def_config) override
    {
    }

  protected:
    void SetChoosingControls(ATTRIBUTES *pA);

  protected:
    int32_t *m_pControlsID;
    bool *m_pbControlsStick;
    int32_t m_keysQuantity;
    bool m_bKeyCheck;
};
