#include "xservice.h"

#include "dx9render.h"
#include "string_compare.hpp"

#include "Filesystem/Config/Config.hpp"
#include "Filesystem/Constants/ConfigNames.hpp"

#define ERROR_MUL 1.0f

XSERVICE::XSERVICE()
    : m_pRS(nullptr),
    m_fWScale(0),
    m_fHScale(0),
    m_fWAdd(0),
    m_fHAdd(0) {
}

void XSERVICE::Init(VDX9RENDER *pRS, int32_t lWidth, int32_t lHeight)
{
    m_pRS = pRS;

    // get the size of the output window
    /*    D3DVIEWPORT9 vp;
      m_pRS->GetViewport(&vp);
      m_fWAdd = m_fWScale = (float)vp.Width/lWidth;
      m_fHAdd = m_fHScale = (float)vp.Height/lHeight;

      while(m_fWAdd>1.f) m_fWAdd-=1.f;
      while(m_fWAdd<0.f) m_fWAdd+=1.f;
      while(m_fHAdd>1.f) m_fHAdd-=1.f;
      while(m_fHAdd<0.f) m_fHAdd+=1.f;
      if(m_fWAdd>0.5f) m_fWAdd = 2.f-m_fWAdd*2.f;
      else    m_fWAdd *= 2.f;
      if(m_fHAdd>0.5f) m_fHAdd = 2.f-m_fHAdd*2.f;
      else    m_fHAdd *= 2.f;
      m_fWAdd *= ERROR_MUL;
      m_fHAdd *= ERROR_MUL;
    */
    m_fWAdd = 0.5f;
    m_fHAdd = 0.5f;

    LoadAllPicturesInfo();
}

int32_t XSERVICE::GetTextureID(const char *sImageListName) {
    if (sImageListName == nullptr) {
        return -1;
    }
    for (auto i = 0; i < std::size(m_pList); i++) {
        if (m_pList[i].sImageListName != std::string(sImageListName)) {
            continue;
        }
        if (m_pList[i].textureQuantity == 0) {
            m_pList[i].textureID = m_pRS->TextureCreate(std::string("INTERFACES\\" + m_pList[i].sTextureName).c_str());
            m_pList[i].textureQuantity = 1;
        }
        else {
            m_pList[i].textureQuantity++;
        }
        return m_pList[i].textureID;
    }

    return -1;
}

int32_t XSERVICE::FindGroup(const char *sImageListName) const {
    std::int32_t idx = -1;
    if (sImageListName == nullptr) {
        return idx;
    }
    for (auto i = 0; i < std::size(m_pList); i++)
        if (m_pList[i].sImageListName == std::string(sImageListName)) {
            idx = i;
            break;
        }
    return idx;
}

bool XSERVICE::ReleaseTextureID(const char *sImageListName) {
    if (sImageListName == nullptr) {
        return false;
    }

    const auto it = std::ranges::find_if(m_pList,
        [&sImageListName](auto &image_descr) {
            return image_descr.sImageListName == std::string(sImageListName);
        });
    if (it == std::end(m_pList) || it->textureQuantity == 0) {
        return false;
    }
    m_pRS->TextureRelease(it->textureID);
    it->textureQuantity--;
    return false;
}

bool XSERVICE::GetTexturePos(int32_t pictureNum, FXYRECT &texRect) {
    return GetTexturePos(0, pictureNum, texRect);
}

bool XSERVICE::GetTexturePos(int32_t pictureNum, XYRECT &texRect) {
    texRect = XYRECT();
    if (static_cast<std::size_t>(pictureNum) > std::size(m_pImage)) {
        return false;
    }
    texRect = m_pImage[pictureNum].pTextureRect;
    return true;
}

bool XSERVICE::GetTexturePos(const char *sImageListName, const char *sImageName, FXYRECT &texRect)
{
    return GetTexturePos(GetImageNum(sImageListName, sImageName), texRect);
}

bool XSERVICE::GetTexturePos(const char *sImageListName, const char *sImageName, XYRECT &texRect)
{
    return GetTexturePos(GetImageNum(sImageListName, sImageName), texRect);
}

bool XSERVICE::GetTexturePos(int nTextureModify, int32_t pictureNum, FXYRECT &texRect) {
    texRect = FXYRECT();
    if (static_cast<std::size_t>(pictureNum) > std::size(m_pImage)) {
        return false;
    }
    // find picture group
    const auto picture_it = std::ranges::find_if(m_pList,
        [&pictureNum](auto& picture_descr) {
            return pictureNum >= picture_descr.pictureStart
                && pictureNum < picture_descr.pictureStart + picture_descr.pictureQuantity;
        });
    if (picture_it == std::end(m_pList)) {
        return false;
    }
    texRect.left = static_cast<float>(m_pImage[pictureNum].pTextureRect.left);
    texRect.top = static_cast<float>(m_pImage[pictureNum].pTextureRect.top);
    texRect.right = static_cast<float>(m_pImage[pictureNum].pTextureRect.right);
    texRect.bottom = static_cast<float>(m_pImage[pictureNum].pTextureRect.bottom);
    if (nTextureModify & TEXTURE_MODIFY_HORZFLIP)
    {
        const auto tmp = texRect.left + m_fWAdd * 2.f;
        texRect.left = texRect.right - m_fWAdd * 2.f;
        texRect.right = tmp;
    }
    if (nTextureModify & TEXTURE_MODIFY_VERTFLIP)
    {
        const auto tmp = texRect.top + m_fHAdd * 2.f;
        texRect.top = texRect.bottom - m_fHAdd * 2.f;
        texRect.bottom = tmp;
    }
    texRect.left = (texRect.left + m_fWAdd) / picture_it->textureWidth;
    texRect.right = (texRect.right - m_fWAdd) / picture_it->textureWidth;
    texRect.top = (texRect.top + m_fHAdd) / picture_it->textureHeight;
    texRect.bottom = (texRect.bottom - m_fHAdd) / picture_it->textureHeight;
    return true;
}

bool XSERVICE::GetTexturePos(int nTextureModify, const char *sImageListName, const char *sImageName, FXYRECT &texRect)
{
    return GetTexturePos(nTextureModify, GetImageNum(sImageListName, sImageName), texRect);
}

void XSERVICE::GetTextureCutForSize(const char *pcImageListName, const FXYPOINT &pntLeftTopUV, const XYPOINT &pntSize,
                                    int32_t nSrcWidth, int32_t nSrcHeight, FXYRECT &outUV)
{
    const auto n = FindGroup(pcImageListName);
    if (n > -1) {
        if (nSrcWidth < m_pList[n].textureWidth) {
            nSrcWidth = m_pList[n].textureWidth;
        }
        if (nSrcHeight < m_pList[n].textureHeight) {
            nSrcHeight = m_pList[n].textureHeight;
        }
    }
    auto fW = nSrcWidth > 0 ? static_cast<float>(pntSize.x) / nSrcWidth + pntLeftTopUV.x : 1.f;
    auto fH = nSrcHeight > 0 ? static_cast<float>(pntSize.y) / nSrcHeight + pntLeftTopUV.y : 1.f;
    fW = std::clamp(fW, 0.f, 1.f);
    fH = std::clamp(fH, 0.f, 1.f);
    outUV.left = pntLeftTopUV.x;
    outUV.top = pntLeftTopUV.y;
    outUV.right = fW;
    outUV.bottom = fH;
}

void XSERVICE::LoadAllPicturesInfo() {
    auto config = Storm::Filesystem::Config::Load(Storm::Filesystem::Constants::ConfigNames::pictures());
    const auto sections = config.Sections();

    m_pList.reserve(std::size(sections));
    for (int i = 0; i < std::size(sections); i++) {
        std::ignore = config.SelectSection(sections[i]);
        m_pList.emplace_back();
        m_pList[i].sImageListName = sections[i];
        m_pList[i].sTextureName = config.Get<std::string>("sTextureName", {});
        m_pList[i].textureWidth = config.Get<std::int64_t>("wTextureWidth", 1024);
        m_pList[i].textureHeight = config.Get<std::int64_t>("wTextureHeight", 1024);

        auto pictures = config.Get<std::vector<std::vector<std::string>>>("picture", {});

        m_pList[i].pictureStart = std::size(m_pImage);

        if (std::size(pictures[0]) == 1) {
            pictures = {{pictures[0][0], pictures[1][0], pictures[2][0], pictures[3][0], pictures[4][0]}};
        }

        m_pList[i].pictureQuantity = std::size(pictures);

        for (auto & picture : pictures) {
            PICTUREDESCR picture_descr;
            picture_descr.sPictureName = picture[0];
            picture_descr.pTextureRect.left = std::stoi(picture[1]);
            picture_descr.pTextureRect.top = std::stoi(picture[2]);
            picture_descr.pTextureRect.right = std::stoi(picture[3]);
            picture_descr.pTextureRect.bottom = std::stoi(picture[4]);
            m_pImage.emplace_back(picture_descr);
        }
    }
}

void XSERVICE::ReleaseAll()
{
    for (const auto &each : m_pList) {
        if (each.textureQuantity != 0) {
            m_pRS->TextureRelease(each.textureID);
        }
    }
    m_pList.clear();
    m_pImage.clear();
}

int32_t XSERVICE::GetImageNum(const char *sImageListName, const char *sImageName) {
    int32_t retVal = -1;

    if (sImageName == nullptr) {
        return retVal;
    }

    std::size_t start_offset = 0;
    std::size_t end_offset = std::size(m_pImage);
    if (sImageListName != nullptr) {
        const auto picture_it = std::ranges::find_if(m_pList,
                                       [&sImageListName](const IMAGELISTDESCR& image_descr) {
                                           return image_descr.sImageListName == std::string(sImageListName);
                                       });
        if (picture_it != std::end(m_pList)) {
            start_offset = picture_it->pictureStart;
            end_offset = picture_it->pictureQuantity;
        }
    }
    const auto image_it = std::find_if(std::begin(m_pImage) + start_offset,
        std::begin(m_pImage) + start_offset + end_offset,
                                   [&sImageName](const PICTUREDESCR& picture_descr) {
                                       return picture_descr.sPictureName == std::string(sImageName);
                                   });
    if (image_it != std::end(m_pImage)) {
        retVal = std::distance(std::begin(m_pImage), image_it);
    }

    return retVal;
}
