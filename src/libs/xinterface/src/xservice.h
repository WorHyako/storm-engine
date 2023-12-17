#pragma once

#include "vx_service.h"
#include "xdefines.h"

class XSERVICE final : public VXSERVICE
{
    struct IMAGELISTDESCR
    {
        char *sImageListName;
        char *sTextureName;
        int32_t textureID;
        int textureQuantity;

        int32_t textureWidth;
        int32_t textureHeight;
        int32_t pictureQuantity;
        int32_t pictureStart;
    };

    struct PICTUREDESCR
    {
        char *sPictureName;
        XYRECT pTextureRect;
    };

  public:
    explicit XSERVICE(VDX9RENDER *pRS);
    ~XSERVICE() override;

    // get texture identificator for image group
    int32_t GetTextureID(const std::string_view &sImageListName) override;
    int32_t FindGroup(const std::string_view &sImageListName) const;
    bool ReleaseTextureID(const std::string_view &sImageListName) override;

    // get texture positon for select picture
    bool GetTexturePos(int32_t pictureNum, FXYRECT &texRect) override;
    bool GetTexturePos(int32_t pictureNum, XYRECT &texRect) override;
    bool GetTexturePos(const std::string_view &sImageListName, const std::string_view &sImageName,
                       FXYRECT &texRect) override;
    bool GetTexturePos(const std::string_view &sImageListName, const std::string_view &sImageName,
                       XYRECT &texRect) override;
    bool GetTexturePos(int nTextureModify, int32_t pictureNum, FXYRECT &texRect) override;
    bool GetTexturePos(int nTextureModify, const std::string_view &sImageListName, const std::string_view &sImageName,
                       FXYRECT &texRect) override;

    void GetTextureCutForSize(const std::string_view &pcImageListName, const FXYPOINT &pntLeftTopUV,
                              const XYPOINT &pntSize, int32_t nSrcWidth, int32_t nSrcHeight, FXYRECT &outUV) override;

    int32_t GetImageNum(const std::string_view &sImageListName, const std::string_view &sImageName) override;

  private:
    void LoadAllPicturesInfo();

    VDX9RENDER *m_pRS = nullptr;

    int32_t m_dwListQuantity = 0;
    int32_t m_dwImageQuantity = 0;
    IMAGELISTDESCR *m_pList = nullptr;
    PICTUREDESCR *m_pImage = nullptr;

    // Scale factors
    float m_fWScale = 0;
    float m_fHScale = 0;
    // scaling error parameters
    float m_fWAdd = 0.5f;
    float m_fHAdd = 0.5f;
};
