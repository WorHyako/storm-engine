#pragma once

#include <cstdint>
#include <string>
#include <vector>
/**
 *TODO: header
 */
#include <span>

class MapZipper
{
  public:
    [[nodiscard]] uint32_t GetSizeX() const;

    void DoZip(const std::span<uint8_t> &pSrc, uint32_t dwSizeX);
    [[nodiscard]] uint8_t Get(uint32_t dwX, uint32_t dwY) const;

    bool Save(std::string sFileName) const;
    bool Load(std::string sFileName);

  private:
    std::vector<uint16_t> wordTable_;
    std::vector<uint8_t> realData_;

    uint32_t dwSizeX{};
    uint32_t dwDX{};
    uint32_t dwBlockSize{};
    uint32_t dwBlockShift{};
    uint32_t dwShiftNumBlocksX{};
    uint32_t dwNumRealBlocks{};
};