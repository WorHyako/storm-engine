#pragma once

#include "vx_service.h"
#include "xdefines.h"

class XSERVICE : public VXSERVICE
{
    struct IMAGELISTDESCR
    {
        std::string sImageListName;
        std::string sTextureName;
        std::int32_t textureID = -1L;
        std::int32_t textureQuantity = 0;

        std::int32_t textureWidth = 1024;
        std::int32_t textureHeight = 1024;
        std::int32_t pictureQuantity = 0;
        std::int32_t pictureStart = 0;
    };

    struct PICTUREDESCR
    {
        std::string sPictureName;
        XYRECT pTextureRect;
    };

  public:
    XSERVICE();
    ~XSERVICE() override = default;

    // initialization of service
    void Init(VDX9RENDER *pRS, int32_t lWidth, int32_t lHight) override;

    // get texture identificator for image group
    int32_t GetTextureID(const char *sImageListName) override;
    int32_t FindGroup(const char *sImageListName) const;
    bool ReleaseTextureID(const char *sImageListName) override;

    // get texture positon for select picture
    bool GetTexturePos(int32_t pictureNum, FXYRECT &texRect) override;
    bool GetTexturePos(int32_t pictureNum, XYRECT &texRect) override;
    bool GetTexturePos(const char *sImageListName, const char *sImageName, FXYRECT &texRect) override;
    bool GetTexturePos(const char *sImageListName, const char *sImageName, XYRECT &texRect) override;
    bool GetTexturePos(int nTextureModify, int32_t pictureNum, FXYRECT &texRect) override;
    bool GetTexturePos(int nTextureModify, const char *sImageListName, const char *sImageName,
                       FXYRECT &texRect) override;

    void GetTextureCutForSize(const char *pcImageListName, const FXYPOINT &pntLeftTopUV, const XYPOINT &pntSize,
                              int32_t nSrcWidth, int32_t nSrcHeight, FXYRECT &outUV) override;

    int32_t GetImageNum(const char *sImageListName, const char *sImageName) override;

    void ReleaseAll() override;

  protected:
    void LoadAllPicturesInfo();

  protected:
    VDX9RENDER *m_pRS;

    std::vector<IMAGELISTDESCR> m_pList;
    std::vector<PICTUREDESCR> m_pImage;

    // Scale factors
    float m_fWScale;
    float m_fHScale;
    // scaling error parameters
    float m_fWAdd;
    float m_fHAdd;
};
