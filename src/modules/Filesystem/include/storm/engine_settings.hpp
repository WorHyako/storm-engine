#include <filesystem>
#include <optional>

namespace storm {

enum class EngineSettingsPathType
{
    Stash,
    Logs,
    SaveData,
    Screenshots,
    ScriptCache,
    Sentry,
};

class EngineSettings {
  public:
    std::filesystem::path GetEnginePath(EngineSettingsPathType type);

  private:
    static EngineSettings Create();

    std::filesystem::path stashFolder_;
    std::optional<std::filesystem::path> logsFolder_;

    friend EngineSettings &GetEngineSettings();
};

EngineSettings &GetEngineSettings();

} // namespace storm