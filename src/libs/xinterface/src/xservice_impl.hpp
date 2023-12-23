#pragma once

#include "xservice.hpp"

#include "dx9render.h"
#include "string_compare.hpp"
#include "v_file_service.h"

static constexpr const char *LISTS_INIFILE = R"(resource\ini\interfaces\pictures.ini)";

template<typename T>
T GetNumber(const storm::Data &data, const std::string &name, const T def)
{
    if (data.contains(name)) {
        if (data[name].is_number()) {
            return data[name].get<T>();
        }
        else if (data[name].is_string()) {
            return static_cast<T>(std::stod(data[name].get<std::string>()));
        }
    }
    return def;
}

std::string GetString(const storm::Data &data, const std::string &def)
{
    if (data.is_string())
    {
        return data.get<std::string>();
    }
    else if (data.is_array() && !data.empty() && data[0].is_string())
    {
        return data[0].get<std::string>();
    }
    return def;
}

std::string GetString(const storm::Data &data, const std::string &name, const std::string &def)
{
    if (data.contains(name))
    {
        return GetString(data[name], def);
    }
    return def;
}

template<TexturePoolConcept TexturePool>
XSERVICE<TexturePool>::XSERVICE(TexturePool &pRS)
    : m_pRS(pRS)
{
    // initialize ini file
    auto ini = fio->OpenIniFile(LISTS_INIFILE);
    if (!ini)
    {
        throw std::runtime_error("ini file not found!");
    }

    LoadAllPicturesInfo(ini->ToData());
}

template<TexturePoolConcept TexturePool>
XSERVICE<TexturePool>::XSERVICE(TexturePool &pRS, const storm::Data &config)
    : m_pRS(pRS)
{
    LoadAllPicturesInfo(config);
}



template<TexturePoolConcept TexturePool>
XSERVICE<TexturePool>::~XSERVICE()
{
    for (const auto &image_desc : m_pList) {
        if (image_desc.textureQuantity != 0)
        {
            m_pRS.TextureRelease(image_desc.textureID);
        }
    }
}

template<TexturePoolConcept TexturePool>
int32_t XSERVICE<TexturePool>::GetTextureID(const std::string_view &sImageListName)
{
    for (auto &image_desc : m_pList) {
        if (storm::iEquals(image_desc.sImageListName, sImageListName))
        {
            if (image_desc.textureQuantity <= 0)
            {
                std::string texture_name = std::format("INTERFACES\\{}", image_desc.sTextureName);
                image_desc.textureID = m_pRS.TextureCreate(texture_name.c_str() );
                image_desc.textureQuantity = 1;
            }
            else
            {
                image_desc.textureQuantity++;
            }
            return image_desc.textureID;
        }
    }

    return -1;
}

template<TexturePoolConcept TexturePool>
int32_t XSERVICE<TexturePool>::FindGroup(const std::string_view &sImageListName) const
{
    for (auto n = 0; n < m_pList.size(); n++)
    {
        if (storm::iEquals(m_pList[n].sImageListName, sImageListName))
        {
            return n;
        }
    }
    return -1;
}

template<TexturePoolConcept TexturePool>
bool XSERVICE<TexturePool>::ReleaseTextureID(const std::string_view &sImageListName)
{
    for (auto i = 0; i < m_pList.size(); i++)
    {
        if (storm::iEquals(m_pList[i].sImageListName, sImageListName))
        {
            if (--m_pList[i].textureQuantity == 0)
            {
                m_pRS.TextureRelease(m_pList[i].textureID);
                return true;
            }
        }
    }

    return false;
}

template<TexturePoolConcept TexturePool>
bool XSERVICE<TexturePool>::GetTexturePos(int32_t pictureNum, FXYRECT &texRect)
{
    if (pictureNum >= 0 && pictureNum < m_pImage.size())
    {
        // find picture group
        int gn;
        for (gn = 0; gn < m_pList.size(); gn++)
        {
            if (pictureNum >= m_pList[gn].pictureStart &&
                pictureNum < m_pList[gn].pictureStart + m_pList[gn].pictureQuantity)
            {
                break;
            }
        }
        if (gn < m_pList.size())
        {
            texRect.left =
                static_cast<float>(m_pImage[pictureNum].pTextureRect.left + m_fWAdd) / m_pList[gn].textureWidth;
            texRect.right =
                static_cast<float>(m_pImage[pictureNum].pTextureRect.right - m_fWAdd) / m_pList[gn].textureWidth;
            texRect.top =
                static_cast<float>(m_pImage[pictureNum].pTextureRect.top + m_fHAdd) / m_pList[gn].textureHeight;
            texRect.bottom =
                static_cast<float>(m_pImage[pictureNum].pTextureRect.bottom - m_fHAdd) / m_pList[gn].textureHeight;
            return true;
        }
    }

    texRect = {};
    return false;
}

template<TexturePoolConcept TexturePool>
bool XSERVICE<TexturePool>::GetTexturePos(int32_t pictureNum, XYRECT &texRect)
{
    if (pictureNum >= 0 && pictureNum < m_pImage.size())
    {
        memcpy(&texRect, &m_pImage[pictureNum].pTextureRect, sizeof(XYRECT));
        return true;
    }

    texRect = {};
    return false;
}

template<TexturePoolConcept TexturePool>
bool XSERVICE<TexturePool>::GetTexturePos(const std::string_view &sImageListName, const std::string_view &sImageName,
                             FXYRECT &texRect)
{
    return GetTexturePos(GetImageNum(sImageListName, sImageName), texRect);
}

template<TexturePoolConcept TexturePool>
bool XSERVICE<TexturePool>::GetTexturePos(const std::string_view &sImageListName, const std::string_view &sImageName,
                             XYRECT &texRect)
{
    return GetTexturePos(GetImageNum(sImageListName, sImageName), texRect);
}

template<TexturePoolConcept TexturePool>
bool XSERVICE<TexturePool>::GetTexturePos(int nTextureModify, int32_t pictureNum, FXYRECT &texRect)
{
    FXYRECT rectTmp;

    if (pictureNum >= 0 && pictureNum < m_pImage.size())
    {
        // find picture group
        int gn;
        for (gn = 0; gn < m_pList.size(); gn++)
        {
            if ((pictureNum >= m_pList[gn].pictureStart) &&
                (pictureNum < m_pList[gn].pictureStart + m_pList[gn].pictureQuantity))
            {
                break;
            }
        }
        if (gn < m_pList.size())
        {
            rectTmp.left = static_cast<float>(m_pImage[pictureNum].pTextureRect.left);
            rectTmp.top = static_cast<float>(m_pImage[pictureNum].pTextureRect.top);
            rectTmp.right = static_cast<float>(m_pImage[pictureNum].pTextureRect.right);
            rectTmp.bottom = static_cast<float>(m_pImage[pictureNum].pTextureRect.bottom);
            if (nTextureModify & TEXTURE_MODIFY_HORZFLIP)
            {
                const auto tmp = rectTmp.left + m_fWAdd * 2.f;
                rectTmp.left = rectTmp.right - m_fWAdd * 2.f;
                rectTmp.right = tmp;
            }
            if (nTextureModify & TEXTURE_MODIFY_VERTFLIP)
            {
                const auto tmp = rectTmp.top + m_fHAdd * 2.f;
                rectTmp.top = rectTmp.bottom - m_fHAdd * 2.f;
                rectTmp.bottom = tmp;
            }
            texRect.left = (rectTmp.left + m_fWAdd) / m_pList[gn].textureWidth;
            texRect.right = static_cast<float>(rectTmp.right - m_fWAdd) / m_pList[gn].textureWidth;
            texRect.top = (rectTmp.top + m_fHAdd) / m_pList[gn].textureHeight;
            texRect.bottom = static_cast<float>(rectTmp.bottom - m_fHAdd) / m_pList[gn].textureHeight;
            return true;
        }
    }

    texRect = {};
    return false;
}

template<TexturePoolConcept TexturePool>
bool XSERVICE<TexturePool>::GetTexturePos(int nTextureModify, const std::string_view &sImageListName,
                             const std::string_view &sImageName, FXYRECT &texRect)
{
    return GetTexturePos(nTextureModify, GetImageNum(sImageListName, sImageName), texRect);
}

template<TexturePoolConcept TexturePool>
void XSERVICE<TexturePool>::GetTextureCutForSize(const std::string_view &pcImageListName, const FXYPOINT &pntLeftTopUV,
                                    const XYPOINT &pntSize, int32_t nSrcWidth, int32_t nSrcHeight, FXYRECT &outUV)
{
    const auto n = FindGroup(pcImageListName);
    if (n >= 0)
    {
        if (nSrcWidth < m_pList[n].textureWidth)
        {
            nSrcWidth = m_pList[n].textureWidth;
        }
        if (nSrcHeight < m_pList[n].textureHeight)
        {
            nSrcHeight = m_pList[n].textureHeight;
        }
    }
    auto fW = 1.f;
    if (nSrcWidth > 0)
    {
        fW = static_cast<float>(pntSize.x) / nSrcWidth + pntLeftTopUV.x;
    }
    auto fH = 1.f;
    if (nSrcHeight > 0)
    {
        fH = static_cast<float>(pntSize.y) / nSrcHeight + pntLeftTopUV.y;
    }
    if (fW > 1.f)
    {
        fW = 1.f;
    }
    if (fH > 1.f)
    {
        fH = 1.f;
    }
    outUV.left = pntLeftTopUV.x;
    outUV.top = pntLeftTopUV.y;
    outUV.right = fW;
    outUV.bottom = fH;
}

template<TexturePoolConcept TexturePool>
void XSERVICE<TexturePool>::LoadAllPicturesInfo(const storm::Data &config)
{
    char param[255];

    int32_t image_quantity = 0;

    for (const auto& [section, table] : config.items()) {
        auto& picture_list = m_pList.emplace_back(IMAGELISTDESCR{
            .sImageListName = section,
            .sTextureName = GetString(table, "sTextureName", ""),
            .textureID = -1,
            .textureQuantity = 0,
            .textureWidth = GetNumber<int32_t>(table, "wTextureWidth", 1024),
            .textureHeight = GetNumber<int32_t>(table, "wTextureHeight", 1024),
            .pictureQuantity = 0,
            .pictureStart = image_quantity,
        });

        if (table.contains("picture") ) {
            const auto& picture_data = table["picture"];
            if (picture_data.is_array() ) {
                for (const auto &[_, picture_entry] : picture_data.items()) {
                    ++image_quantity;
                    ++picture_list.pictureQuantity;

                    const std::string &picture_string = GetString(picture_entry, "");
                    char picName[sizeof(param)];
                    // get texture coordinates
                    int nLeft, nTop, nRight, nBottom;

                    sscanf(picture_string.c_str(), "%[^,],%d,%d,%d,%d", picName, &nLeft, &nTop, &nRight, &nBottom);
                    m_pImage.emplace_back(PICTUREDESCR{
                        .sPictureName = picName,
                        .pTextureRect = {
                            nLeft,
                            nTop,
                            nRight,
                            nBottom
                        },
                    });
                }
            }
            else if (picture_data.is_string() ) {
                ++image_quantity;
                ++picture_list.pictureQuantity;

                const std::string &picture_string = picture_data.get<std::string>();

                char picName[sizeof(param)];
                // get texture coordinates
                int nLeft, nTop, nRight, nBottom;

                sscanf(picture_string.c_str(), "%[^,],%d,%d,%d,%d", picName, &nLeft, &nTop, &nRight, &nBottom);
                m_pImage.emplace_back(PICTUREDESCR{
                    .sPictureName = picName,
                    .pTextureRect = {
                        nLeft,
                        nTop,
                        nRight,
                        nBottom
                    },
                });
            }
        }

    }
}

template<TexturePoolConcept TexturePool>
int32_t XSERVICE<TexturePool>::GetImageNum(const std::string_view &sImageListName, const std::string_view &sImageName)
{
    int32_t retVal = -1;

    if (!sImageListName.empty())
    {
        for (auto &i : m_pList)
        {
            if (storm::iEquals(i.sImageListName, sImageListName))
            {
                for (int j = i.pictureStart; j < i.pictureStart + i.pictureQuantity; j++)
                {
                    if (storm::iEquals(m_pImage[j].sPictureName, sImageName))
                    {
                        retVal = j;
                        break;
                    }
                }
                break;
            }
        }
    }
    else
    {
        for (int i = 0; i < m_pImage.size(); i++)
        {
            if (storm::iEquals(m_pImage[i].sPictureName, sImageName))
            {
                retVal = i;
                break;
            }
        }
    }

    return retVal;
}

