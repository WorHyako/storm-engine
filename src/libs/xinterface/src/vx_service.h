#pragma once

#include <cstdint>
#include <string_view>

#define TEXTURE_MODIFY_NONE 0
#define TEXTURE_MODIFY_HORZFLIP 1
#define TEXTURE_MODIFY_VERTFLIP 2

class VDX9RENDER;
struct FXYRECT;
struct XYRECT;
struct FXYPOINT;
struct XYPOINT;

class VXSERVICE
{
  public:
    virtual ~VXSERVICE() = default;
    virtual void Init(VDX9RENDER *pRS, int32_t lWidth, int32_t lHight) = 0;

    // get texture identificator for image group
    virtual int32_t GetTextureID(const std::string_view &sImageListName) = 0;
    virtual bool ReleaseTextureID(const std::string_view &sImageListName) = 0;

    // get texture positon for select picture
    virtual bool GetTexturePos(int32_t pictureNum, FXYRECT &texRect) = 0;
    virtual bool GetTexturePos(int32_t pictureNum, XYRECT &texRect) = 0;
    virtual bool GetTexturePos(const std::string_view &sImageListName, const std::string_view &sImageName, FXYRECT &texRect) = 0;
    virtual bool GetTexturePos(const std::string_view &sImageListName, const std::string_view &sImageName, XYRECT &texRect) = 0;
    virtual bool GetTexturePos(int nTextureModify, int32_t pictureNum, FXYRECT &texRect) = 0;
    virtual bool GetTexturePos(int nTextureModify, const std::string_view &sImageListName, const std::string_view &sImageName,
                               FXYRECT &texRect) = 0;

    virtual void GetTextureCutForSize(const std::string_view &pcImageListName, const FXYPOINT &pntLeftTopUV, const XYPOINT &pntSize,
                                      int32_t nSrcWidth, int32_t nSrcHeight, FXYRECT &outUV) = 0;

    virtual int32_t GetImageNum(const std::string_view &sImageListName, const std::string_view &sImageName) = 0;

    virtual void ReleaseAll() = 0;
};
