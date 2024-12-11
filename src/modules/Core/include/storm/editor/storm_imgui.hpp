#pragma once

#include <fmt/format.h>
#include <imgui.h>
#include <imgui_stdlib.h>

#include <string>
#include <utility>

namespace ImGui
{

template <typename T, typename... Args>
IMGUI_API void TextFmt(T &&fmt, const Args &...args)
{
    std::string str = fmt::format(fmt::runtime(std::forward<T>(fmt)), args...);
    ImGui::TextUnformatted(str.c_str(), str.c_str() + str.length() );
}

}
