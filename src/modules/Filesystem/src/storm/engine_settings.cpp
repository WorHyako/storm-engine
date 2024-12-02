#include "storm/engine_settings.hpp"
#include "storm/config/config.hpp"

#include <fs.h>

namespace storm
{

EngineSettings &GetEngineSettings()
{
    static EngineSettings settings = EngineSettings::Create();
    return settings;
}

std::filesystem::path EngineSettings::GetEnginePath(EngineSettingsPathType type)
{
    switch (type)
    {
    case EngineSettingsPathType::Stash: {
        return stashFolder_;
    }
    case EngineSettingsPathType::Logs: {
        return logsFolder_.value_or(stashFolder_ / "Logs");
    }
    case EngineSettingsPathType::SaveData: {
        return stashFolder_ / "SaveData";
    }
    case EngineSettingsPathType::Screenshots: {
        return stashFolder_ / "Screenshots";
    }
    case EngineSettingsPathType::ScriptCache: {
        return stashFolder_ / "Cache";
    }
    case EngineSettingsPathType::Sentry: {
        return stashFolder_ / "sentry-db";
    }
    default:
        throw std::runtime_error("Unknown engine settings path");
    }
}

EngineSettings EngineSettings::Create()
{
    EngineSettings result;
    const Data config = *LoadConfig("engine");
    if (config.contains("stash_folder") ) {
        const auto stash_folder = config.value<std::string>("stash_folder", "stash");
        result.stashFolder_ = std::filesystem::path(stash_folder);
    }
    else {
        result.stashFolder_ = fs::GetDefaultStashPath().value_or(std::filesystem::path("stash"));
    }

    if (config.contains("logs_folder") ) {
        const auto logs_folder = config.value<std::string>("logs_folder", ".");
        result.logsFolder_ = std::filesystem::path(logs_folder);
    }
    return result;
}

} // namespace storm