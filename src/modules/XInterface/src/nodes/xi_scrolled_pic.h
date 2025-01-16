#pragma once

#include "xi_image.h"
#include "xi_picture.h"

class CXI_SCROLLEDPICTURE : public CXI_PICTURE
{
  public:
    CXI_SCROLLEDPICTURE();
    ~CXI_SCROLLEDPICTURE() override;
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
    void MoveMouseOutScreen(float fX, float fY) override;
    void ChangeUV(FXYRECT &frNewUV) override;

  protected:
    void LoadIni(const Storm::Filesystem::Config& node_config, const Storm::Filesystem::Config& def_config) override;
    void SetNewPicture(bool video, char *sNewTexName);
    void SetNewPictureFromDir(char *dirName);
    void RecalculateTexPerPixel();
    void UpdateBuildenImages();
    void SetPosToCenter(float fX, float fY);
    void SetScale(int32_t nScaleIdx);
    void SetScale(float fsx, float fsy);

    float m_fUTexPerPixel;
    float m_fVTexPerPixel;

    FXYPOINT m_fpBaseSize;

    struct BuildinImage
    {
        CXI_IMAGE *pImg;
        FXYPOINT fpPos;
        FXYPOINT fpSize;
        bool bShow;
    };

    int32_t m_nScaleNum;
    std::vector<FXYPOINT> m_aScale;
    std::vector<BuildinImage> m_aImg;
};
