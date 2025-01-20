#include "font_loading.hpp"

#include "bmfont/bmfont.hpp"
#include "core.h"
#include <storm/config/config.hpp>

#include "Filesystem/Config/Config.hpp"
#include "Filesystem/Constants/ConfigNames.hpp"

using namespace Storm::Filesystem;
using namespace Storm::Math;

namespace storm {

std::unique_ptr<VFont> LoadFont(const std::string_view &font_name,
                                const std::string_view &ini_file_name,
                                VDX9RENDER &renderer,
                                IDirect3DDevice9 &device)
{
    const auto font_name_str = std::string(font_name);

    const auto opt_config = storm::LoadConfig(ini_file_name);

    if (!opt_config) {
        return nullptr;
    }

    const auto &config = *opt_config;

    if (config.contains(font_name_str) && config[font_name_str].is_object() && config[font_name_str].contains("file")) {
        const auto &section = config[font_name_str];
        auto msg = fmt::format("Loading BmFont {}", font_name);
        core.Trace(msg.c_str());
        std::string fnt_path = section["file"];
        auto result = std::make_unique<bmfont::BmFont>(fnt_path, renderer);

        if (section.contains("pcscale") )
        {
            if (const auto &scale = section["pcscale"]; scale.is_string()) {
                result->SetScale(std::stod(scale.get<std::string>()));
            } else if (scale.is_number()) {
                result->SetScale(scale.get<double>());
            }
        }
        if (section.contains("line_spacing") )
        {
            if (const auto &scale = section["line_spacing"]; scale.is_string()) {
                result->SetLineScale(std::stod(scale.get<std::string>()));
            } else if (scale.is_number()) {
                result->SetLineScale(scale.get<double>());
            }
        }
        if (section.contains("gradient") )
        {
            if (const auto &gradient = section["gradient"]; gradient.is_string())
            {
                const auto gradient_texture_path = gradient.get<std::string>();
                const uint32_t gradient_texture = renderer.TextureCreate(gradient_texture_path.c_str());
                result->SetGradient(gradient_texture);
                renderer.TextureRelease(gradient_texture);
            }
        }
        return result;
    }
    else {
        auto result = std::make_unique<FONT>(renderer, device);
        if (result->Init(font_name_str.c_str(), std::string(ini_file_name).c_str())) {
            return result;
        }
        else {
            core.Trace("Can't init font %s", font_name_str.c_str());
        }
    }

    return {};
}

} // namespace storm
