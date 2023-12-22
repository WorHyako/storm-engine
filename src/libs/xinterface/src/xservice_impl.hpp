#pragma once

#include "xservice.hpp"

#include "dx9render.h"
#include "string_compare.hpp"
#include "v_file_service.h"

static constexpr const char *LISTS_INIFILE = R"(resource\ini\interfaces\pictures.ini)";

template<TexturePoolConcept TexturePool>
XSERVICE<TexturePool>::XSERVICE(TexturePool &pRS)
    : m_pRS(pRS)
{
    LoadAllPicturesInfo();
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
void XSERVICE<TexturePool>::LoadAllPicturesInfo()
{
    char section[255];
    char param[255];

    // initialize ini file
    auto ini = fio->OpenIniFile(LISTS_INIFILE);
    if (!ini)
    {
        throw std::runtime_error("ini file not found!");
    }

    size_t list_quantity = 0;
    size_t image_quantity = 0;

    // calculate lists quantity
    if (ini->GetSectionName(section, sizeof(section) - 1))
    {
        do
        {
            list_quantity++;
        } while (ini->GetSectionNameNext(section, sizeof(section) - 1));
    }
    m_pList.resize(list_quantity);

    // fill lists
    if (ini->GetSectionName(section, sizeof(section) - 1))
    {
        for (auto i = 0; true; i++)
        {
            m_pList[i].textureQuantity = 0;
            m_pList[i].textureID = -1L;

            // get list name
            m_pList[i].sImageListName = section;
            // get texture name
            ini->ReadString(section, "sTextureName", param, sizeof(param) - 1, "");
            m_pList[i].sTextureName = param;

            // get texture width & height
            m_pList[i].textureWidth = ini->GetInt(section, "wTextureWidth", 1024);
            m_pList[i].textureHeight = ini->GetInt(section, "wTextureHeight", 1024);

            m_pList[i].pictureStart = image_quantity;
            // get pictures quantity
            m_pList[i].pictureQuantity = 0;
            if (ini->ReadString(section, "picture", param, sizeof(param) - 1, ""))
            {
                do
                {
                    m_pList[i].pictureQuantity++;
                } while (ini->ReadStringNext(section, "picture", param, sizeof(param) - 1));
            }

            // resize image list
            m_pImage.resize(image_quantity + m_pList[i].pictureQuantity);
            image_quantity += m_pList[i].pictureQuantity;
            Assert(image_quantity == m_pImage.size() );

            // set pictures
            char picName[sizeof(param)];
            ini->ReadString(section, "picture", param, sizeof(param) - 1, "");
            for (int j = m_pList[i].pictureStart; j < image_quantity; j++)
            {
                // get texture coordinates
                int nLeft, nTop, nRight, nBottom;

                sscanf(param, "%[^,],%d,%d,%d,%d", picName, &nLeft, &nTop, &nRight, &nBottom);
                m_pImage[j].pTextureRect.left = nLeft;
                m_pImage[j].pTextureRect.top = nTop;
                m_pImage[j].pTextureRect.right = nRight;
                m_pImage[j].pTextureRect.bottom = nBottom;
                m_pImage[j].sPictureName = picName;

                ini->ReadStringNext(section, "picture", param, sizeof(param) - 1);
            }

            if (!ini->GetSectionNameNext(section, sizeof(section) - 1))
            {
                break;
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

