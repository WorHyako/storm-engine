#pragma once

#include "vx_service.h"
#include "xdefines.h"

using TextureHandle = int32_t;

template<typename T>
concept TexturePoolConcept = requires(T& t, const char *str, TextureHandle texture_id)
{
    texture_id = t.TextureCreate(str);
    t.TextureRelease(texture_id);
};

template<TexturePoolConcept TexturePool>
class XSERVICE final : public VXSERVICE
{
    struct IMAGELISTDESCR
    {
        std::string sImageListName;
        std::string sTextureName;
        int32_t textureID;
        int textureQuantity;

        int32_t textureWidth;
        int32_t textureHeight;
        int32_t pictureQuantity;
        int32_t pictureStart;
    };

    struct PICTUREDESCR
    {
        std::string sPictureName;
        XYRECT pTextureRect;
    };

  public:
    explicit XSERVICE(TexturePool &pRS);
    XSERVICE(TexturePool &pRS, const storm::Data &config);
    ~XSERVICE() override;

    // get texture identificator for image group
    TextureHandle GetTextureID(const std::string_view &sImageListName) override;
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
    void LoadAllPicturesInfo(const storm::Data &config);

    TexturePool &m_pRS;

    std::vector<IMAGELISTDESCR> m_pList;
    std::vector<PICTUREDESCR> m_pImage;

    // scaling error parameters
    float m_fWAdd = 0.5f;
    float m_fHAdd = 0.5f;
};
