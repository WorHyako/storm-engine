#include "debug-trap.h"
#include "island.h"
#include "math_inlines.h"

namespace
{

uint32_t Number2Shift(uint32_t dwNumber)
{
    for (uint32_t i = 0; i < 31; i++)
    {
        if (static_cast<uint32_t>(1 << i) == dwNumber)
        {
            return i;
        }
    }
    return 0;
}

} // namespace

uint32_t MapZipper::GetSizeX() const
{
    return dwSizeX;
}

void MapZipper::DoZip(const std::span<uint8_t> &pSrc, uint32_t _dwSizeX)
{
    uint32_t i, j, k, x, y, xx, yy;

    uint32_t dwRealIndex = 0;

    dwSizeX = _dwSizeX;

    dwBlockSize = 8;
    dwBlockShift = Number2Shift(dwBlockSize);

    dwDX = dwSizeX >> dwBlockShift;

    dwShiftNumBlocksX = Number2Shift(dwDX);

    wordTable_.resize(dwDX * dwDX);
    realData_.resize(dwSizeX * dwSizeX);
    for (i = 0; i < dwDX * dwDX; i++)
    {
        y = i / dwDX;
        x = i - y * dwDX;
        const auto dwStart = (y << dwBlockShift) * dwSizeX + (x << dwBlockShift);

        auto bTest = true;
        uint8_t byTest;
        for (j = 0; j < dwBlockSize * dwBlockSize; j++)
        {
            yy = j >> dwBlockShift;
            xx = j - (yy << dwBlockShift);
            const auto byRes = pSrc[dwStart + yy * dwSizeX + xx];
            if (j == 0)
            {
                byTest = byRes;
            }
            if (byTest != byRes)
            {
                bTest = false;
                wordTable_[i] = static_cast<uint16_t>(dwRealIndex);
                for (k = 0; k < dwBlockSize * dwBlockSize; k++)
                {
                    yy = k >> dwBlockShift;
                    xx = k - (yy << dwBlockShift);
                    realData_[dwRealIndex * dwBlockSize * dwBlockSize + k] = pSrc[dwStart + yy * dwSizeX + xx];
                }
                dwRealIndex++;
                break;
            }
        }
        if (bTest)
        {
            wordTable_[i] = static_cast<uint16_t>(0x8000) | static_cast<uint16_t>(byTest);
        }
    }
    dwNumRealBlocks = dwRealIndex;
    realData_.resize(dwRealIndex * dwBlockSize * dwBlockSize);

    for (y = 0; y < _dwSizeX; y++)
    {
        for (x = 0; x < _dwSizeX; x++)
        {
            if (Get(x, y) != pSrc[x + y * _dwSizeX])
            {
                psnip_trap();
            }
        }
    }
}

uint8_t MapZipper::Get(uint32_t dwX, uint32_t dwY) const
{
    if (wordTable_.empty())
    {
        return 255;
    }
    const auto wRes = wordTable_[((dwY >> dwBlockShift) << dwShiftNumBlocksX) + (dwX >> dwBlockShift)];
    if (wRes & 0x8000)
    {
        return static_cast<uint8_t>(wRes & 0xFF);
    }
    const auto x = dwX - ((dwX >> dwBlockShift) << dwBlockShift);
    const auto y = dwY - ((dwY >> dwBlockShift) << dwBlockShift);

    const auto byRes =
        realData_[((static_cast<uint32_t>(wRes) << dwBlockShift) << dwBlockShift) + (y << dwBlockShift) + x];

    return byRes;
}

bool MapZipper::Load(std::string sFileName)
{
    auto fileS = fio->_CreateFile(sFileName.c_str(), std::ios::binary | std::ios::in);
    if (!fileS.is_open())
    {
        return false;
    }
    fio->_ReadFile(fileS, &dwSizeX, sizeof(dwSizeX));
    fio->_ReadFile(fileS, &dwDX, sizeof(dwDX));
    fio->_ReadFile(fileS, &dwBlockSize, sizeof(dwBlockSize));
    fio->_ReadFile(fileS, &dwBlockShift, sizeof(dwBlockShift));
    fio->_ReadFile(fileS, &dwShiftNumBlocksX, sizeof(dwShiftNumBlocksX));
    fio->_ReadFile(fileS, &dwNumRealBlocks, sizeof(dwNumRealBlocks));
    wordTable_.resize(dwDX * dwDX);
    fio->_ReadFile(fileS, wordTable_.data(), sizeof(uint16_t) * wordTable_.size());
    realData_.resize(dwNumRealBlocks * dwBlockSize * dwBlockSize);
    fio->_ReadFile(fileS, realData_.data(), sizeof(uint8_t) * realData_.size());
    fio->_CloseFile(fileS);
    return true;
}

bool MapZipper::Save(std::string sFileName) const
{
    auto fileS = fio->_CreateFile(sFileName.c_str(), std::ios::binary | std::ios::out);
    if (!fileS.is_open())
    {
        return false;
    }
    fio->_WriteFile(fileS, &dwSizeX, sizeof(dwSizeX));
    fio->_WriteFile(fileS, &dwDX, sizeof(dwDX));
    fio->_WriteFile(fileS, &dwBlockSize, sizeof(dwBlockSize));
    fio->_WriteFile(fileS, &dwBlockShift, sizeof(dwBlockShift));
    fio->_WriteFile(fileS, &dwShiftNumBlocksX, sizeof(dwShiftNumBlocksX));
    fio->_WriteFile(fileS, &dwNumRealBlocks, sizeof(dwNumRealBlocks));
    fio->_WriteFile(fileS, wordTable_.data(), sizeof(uint16_t) * wordTable_.size());
    fio->_WriteFile(fileS, realData_.data(), sizeof(uint8_t) * realData_.size());
    fio->_CloseFile(fileS);
    return true;
}
