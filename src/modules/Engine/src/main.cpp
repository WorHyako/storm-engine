#include <thread>

#include <SDL.h>
#include <mimalloc-new-delete.h>
#include <mimalloc.h>
#include <spdlog/spdlog.h>
#include <CLI/CLI.hpp>

#include "Filesystem/Constants/Paths.hpp"
#include "Filesystem/Constants/ConfigNames.hpp"

#include "core_private.h"
#include "lifecycle_diagnostics_service.hpp"
#include "logging.hpp"
#include "os_window.hpp"
#include "steam_api.hpp"
#include "v_sound_service.h"
#include "watermark.hpp"

#include "active_perk_shower.h"
#include "ai_sea_goods.h"
#include "animals.h"
#include "animation_service_imp.h"
#include "astronomy.h"
#include "aviplayer/aviplayer.h"
#include "back_scene/back_scene.h"
#include "ball_splash.h"
#include "blood.h"
#include "blots.h"
#include "character_animation_kipper.h"
#include "characters_groups.h"
#include "debug_entity.h"
#include "deck_camera.h"
#include "dialog.hpp"
#include "fader.h"
#include "flag.h"
#include "foam.h"
#include "free_camera.h"
#include "geometry_r.h"
#include "grass.h"
#include "help_chooser/help_chooser.h"
#include "info_handler.h"
#include "interface_manager/interface_manager.h"
#include "island.h"
#include "item_entity/item_entity.h"
#include "k2_wrapper/particles.h"
#include "land/battle_land.h"
#include "land/i_boarding_status.h"
#include "lighter.h"
#include "lightning.h"
#include "lights.h"
#include "lizards.h"
#include "loc_crabs.h"
#include "loc_eagle.h"
#include "loc_rats.h"
#include "location.h"
#include "location_camera.h"
#include "location_effects.h"
#include "location_script_lib.h"
#include "locator.h"
#include "log_and_action.h"
#include "mast.h"
#include "model_realizer.h"
#include "modelr.h"
#include "np_character.h"
#include "pcs_controls.h"
#include "player.h"
#include "rope.h"
#include "s_device.h"
#include "sail.h"
#include "sailors.h"
#include "sailors_editor.h"
#include "scr_shoter/scr_shoter.h"
#include "script_func.h"
#include "script_libriary_test.h"
#include "sea.h"
#include "sea/i_battle.h"
#include "sea/ship_pointer.h"
#include "sea_ai.h"
#include "sea_cameras.h"
#include "sea_operator.h"
#include "seafoam.h"
#include "service/particle_service.h"
#include "sharks.h"
#include "ship.h"
#include "ship_camera.h"
#include "ship_lights.h"
#include "sink_effect.h"
#include "sound.h"
#include "sound_service.h"
#include "spyglass/spyglass.h"
#include "storm/blade/blade.hpp"
#include "storm/config/config_library.hpp"
#include "storm/renderer/scene.hpp"
#include "string_service/obj_str_service.h"
#include "string_service/str_service.h"
#include "sun_glow.h"
#include "teleport.h"
#include "texture_sequence/texture_sequence.h"
#include "timer/timer.h"
#include "tornado.h"
#include "touch.h"
#include "track.h"
#include "vant.h"
#include "vcollide.h"
#include "vma.hpp"
#include "water_flare.h"
#include "water_rings.h"
#include "weather.h"
#include "wide_screen.h"
#include "world_map.h"
#include "world_map_interface/interface.h"
#include "xinterface.h"

#include <steam_api_script_lib.hpp>
#include <Filesystem/Config/Config.hpp>

namespace
{

CorePrivate *core_private;

constexpr char defaultLoggerName[] = "system";
bool isRunning = false;
bool bActive = true;
bool bSoundInBackground = false;

storm::diag::LifecycleDiagnosticsService lifecycleDiagnostics;

void RunFrame()
{
    if (!core_private->Run())
    {
        isRunning = false;
    }

    lifecycleDiagnostics.notifyAfterRun();
}

#ifdef _WIN32
void RunFrameWithOverflowCheck()
{
    __try
    {
        RunFrame();
    }
    __except ([](unsigned code, struct _EXCEPTION_POINTERS *ep) {
        return code == EXCEPTION_STACK_OVERFLOW;
    }(GetExceptionCode(), GetExceptionInformation()))
    {
        _resetstkoflw();
        throw std::runtime_error("Stack overflow");
    }
}
#else
#define RunFrameWithOverflowCheck RunFrame
#endif

void mimalloc_fun(const char *msg, void *arg)
{
    static std::filesystem::path mimalloc_log_path;
    if (mimalloc_log_path.empty())
    {
        mimalloc_log_path = Storm::Filesystem::Constants::Paths::logs() / "mimalloc.log";
        std::error_code ec;
        remove(mimalloc_log_path, ec);
    }

    FILE *mimalloc_log =
#ifdef _MSC_VER
        _wfopen(mimalloc_log_path.c_str(), L"a+b");
#else
        fopen(mimalloc_log_path.c_str(), "a+b");
#endif
    if (mimalloc_log != nullptr)
    {
        fputs(msg, mimalloc_log);
        fclose(mimalloc_log);
    }
}

} // namespace

void HandleWindowEvent(const storm::OSWindow::Event &event)
{
    if (event == storm::OSWindow::Closed)
    {
        isRunning = false;
        if (core_private->initialized())
        {
            core_private->Event("DestroyWindow");
        }
    }
    else if (event == storm::OSWindow::FocusGained)
    {
        bActive = true;
        if (core_private->initialized())
        {
            core_private->AppState(bActive);
            if (const auto soundService = static_cast<VSoundService *>(core.GetService("SoundService"));
                soundService && !bSoundInBackground)
            {
                soundService->SetActiveWithFade(true);
            }
        }
    }
    else if (event == storm::OSWindow::FocusLost)
    {
        bActive = false;
        if (core_private->initialized())
        {
            core_private->AppState(bActive);
            if (const auto soundService = static_cast<VSoundService *>(core.GetService("SoundService"));
                soundService && !bSoundInBackground)
            {
                soundService->SetActiveWithFade(false);
            }
        }
    }
}

CREATE_CLASS(ANIMALS)
CREATE_CLASS(BALLSPLASH)
CREATE_CLASS(BATTLE_INTERFACE)
CREATE_CLASS(ILogAndActions)
CREATE_CLASS(IBoardingStatus)
CREATE_CLASS(BATTLE_LAND_INTERFACE)
CREATE_CLASS(ISPYGLASS)
CREATE_CLASS(SHIPPOINTER)
CREATE_CLASS(ActivePerkShower)
CREATE_CLASS(BITimer)
CREATE_CLASS(ItemEntity)
CREATE_CLASS(WM_INTERFACE)
CREATE_CLASS(BI_InterfaceManager)
CREATE_CLASS(BLADE)
CREATE_CLASS(Blots)
CREATE_CLASS(DIALOG)
CREATE_CLASS(ISLAND)
CREATE_CLASS(CoastFoam)
CREATE_CLASS(Lighter)
CREATE_CLASS(Location)
CREATE_CLASS(NPCharacter)
CREATE_CLASS(Player)
CREATE_CLASS(LocationCamera)
CREATE_CLASS(Fader)
CREATE_CLASS(Grass)
CREATE_CLASS(Lights)
CREATE_CLASS(WideScreen)
CREATE_CLASS(CharacterAnimationKipper)
CREATE_CLASS(LocationEffects)
CREATE_CLASS(CharactersGroups)
CREATE_CLASS(LocEagle)
CREATE_CLASS(Lizards)
CREATE_CLASS(LocRats)
CREATE_CLASS(LocCrabs)
CREATE_CLASS(LocModelRealizer)
CREATE_CLASS(Blood)
CREATE_CLASS(LOCATOR)
CREATE_CLASS(BLAST)
CREATE_CLASS(MAST)
CREATE_CLASS(HULL)
CREATE_CLASS(MODELR)
CREATE_CLASS(PARTICLES)
namespace storm
{
CREATE_CLASS(Scene)
}
CREATE_CLASS(SAIL)
CREATE_CLASS(FLAG)
CREATE_CLASS(ROPE)
CREATE_CLASS(VANT)
CREATE_CLASS(VANTL)
CREATE_CLASS(VANTZ)
CREATE_CLASS(Sailors)
CREATE_CLASS(SailorsEditor)
CREATE_CLASS(SEA)
CREATE_CLASS(AISeaGoods)
CREATE_CLASS(SEA_AI)
CREATE_CLASS(AIFort)
CREATE_CLASS(AIBalls)
CREATE_CLASS(SEA_CAMERAS)
CREATE_CLASS(FREE_CAMERA)
CREATE_CLASS(SHIP_CAMERA)
CREATE_CLASS(DECK_CAMERA)
CREATE_CLASS(Sharks)
CREATE_CLASS(SEAFOAM)
CREATE_CLASS(SEA_OPERATOR)
#include "shadow.h"
CREATE_CLASS(Shadow)

CREATE_CLASS(SHIP)
CREATE_CLASS(ShipLights)
CREATE_CLASS(ShipTracks)
CREATE_CLASS(SOUND)
CREATE_CLASS(SINKEFFECT)
CREATE_CLASS(SoundVisualisationEntity)
CREATE_CLASS(TMPTELEPORT)
CREATE_CLASS(FINDFILESINTODIRECTORY)
CREATE_CLASS(FINDDIALOGNODES)
CREATE_CLASS(Tornado)
CREATE_CLASS(TOUCH)
CREATE_CLASS(WaterRings)
CREATE_CLASS(WEATHER)
CREATE_CLASS(RAIN)
CREATE_CLASS(SUNGLOW)
CREATE_CLASS(LIGHTNING)
CREATE_CLASS(SKY)
CREATE_CLASS(WATERFLARE)
CREATE_CLASS(Astronomy)
CREATE_CLASS(WorldMap)
CREATE_CLASS(OBJ_STRSERVICE)
CREATE_CLASS(TextureSequence)
CREATE_CLASS(XINTERFACE)
CREATE_CLASS(SCRSHOTER)
CREATE_CLASS(HELPCHOOSER)
CREATE_CLASS(InfoHandler)
CREATE_CLASS(CONTROLS_CONTAINER)
CREATE_CLASS(InterfaceBackScene)
CREATE_CLASS(CAviPlayer)

CREATE_SCRIPTLIBRIARY(ScriptLocationLibrary)
CREATE_SCRIPTLIBRIARY(DX9RENDER_SCRIPT_LIBRIARY)
CREATE_SCRIPTLIBRIARY(SCRIPT_LIBRIARY_TEST)
CREATE_SCRIPTLIBRIARY(SCRIPT_RIGGING_FILES)
namespace storm
{
using Config = ConfigLibrary;
CREATE_SCRIPTLIBRIARY(Config)
}
CREATE_SCRIPTLIBRIARY(SCRIPT_INTERFACE_FUNCTIONS)

CREATE_SERVICE(PCS_CONTROLS)
CREATE_SERVICE(DX9RENDER)
CREATE_SERVICE(ParticleService)
CREATE_SERVICE(AnimationServiceImp)
CREATE_SERVICE(COLL)
CREATE_SERVICE(SoundService)
CREATE_SERVICE(LostDeviceSentinel)
CREATE_SERVICE(GEOMETRY)
CREATE_SERVICE(STRSERVICE)
namespace steamapi
{
CREATE_SCRIPTLIBRIARY(SteamApiScriptLib)
}

int main(int argc, char *argv[])
{
    CLI::App app("Storm Engine");

    bool enable_editor = false;
    app.add_flag("--editor", enable_editor, "Enable in-game editor");

    try
    {
        app.parse(argc, argv);
    }
    catch (const CLI::ParseError &e)
    {
        return app.exit(e);
    }

    // Prevent multiple instances
#ifdef _WIN32 // CreateEventA
    if (!CreateEventA(nullptr, false, false, "Global\\FBBD2286-A9F1-4303-B60C-743C3D7AA7BE") ||
        GetLastError() == ERROR_ALREADY_EXISTS)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Another instance is already running!", nullptr);
        return EXIT_SUCCESS;
    }
#endif
    mi_register_output(mimalloc_fun, nullptr);
    mi_option_set(mi_option_show_errors, 1);
    mi_option_set(mi_option_show_stats, 0);
    mi_option_set(mi_option_eager_commit, 1);
    mi_option_set(mi_option_eager_region_commit, 1);
    mi_option_set(mi_option_large_os_pages, 1);
//    mi_option_set(mi_option_page_reset, 0);
//    mi_option_set(mi_option_segment_reset, 0);
    mi_option_set(mi_option_reserve_huge_os_pages, 1);
//    mi_option_set(mi_option_segment_cache, 16);
#ifdef _DEBUG
    mi_option_set(mi_option_verbose, 4);
#endif

    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDL ERROR", "Init Error", nullptr);
        SDL_Log("Something went wrong with SDL Init %s", SDL_GetError());
        return 1;
    }

    // Init diagnostics
    const auto lifecycleDiagnosticsGuard =
#ifdef STORM_ENABLE_CRASH_REPORTS
        lifecycleDiagnostics.initialize(true);
#else
        lifecycleDiagnostics.initialize(false);
#endif
    if (!lifecycleDiagnosticsGuard)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Warning", "Unable to initialize lifecycle service!", nullptr);
    }
    else
    {
        lifecycleDiagnostics.setCrashInfoCollector([]() { core_private->collectCrashInfo(); });
    }

    // Init stash
    std::filesystem::create_directories(Storm::Filesystem::Constants::Paths::save_data());

    // Init logging
    spdlog::set_default_logger(storm::logging::getOrCreateLogger(defaultLoggerName));
    spdlog::info("Logging system initialized. Running on {}", STORM_BUILD_WATERMARK_STRING);
    spdlog::info("mimalloc-redirect status: {}", mi_is_redirected());

    // Init core
    core_private = static_cast<CorePrivate *>(&core);
    core_private->EnableEditor(enable_editor);
    core_private->Init();


    uint32_t dwMaxFPS = 0;
    bool bSteam = false;
    int width = 1024, height = 768;
    int preferred_display = 0;
    bool fullscreen = false;
    bool show_borders = false;
    bool bDebugWindow = false;
    bool bAcceleration = false;
    bool run_in_background = false;
    {
        /**
         * TODO: load config for ALL classes in some place of Engine module once
         */
        auto config =  Storm::Filesystem::Config::load(Storm::Filesystem::Constants::ConfigNames::engine());
        std::ignore = config.select_section("Main");
        dwMaxFPS = config.Get<std::int64_t>("max_fps", 0);
        bDebugWindow = config.Get<std::int64_t>("DebugWindow", 0) == 1;
        bAcceleration = config.Get<std::string>("Acceleration", "0") == "0";

        auto log = config.Get<std::int64_t>("logs", 0);
        if (log == 0) {
            spdlog::set_level(spdlog::level::off);
        }

        width = config.Get<std::int64_t>("screen_x", 1024);
        height = config.Get<std::int64_t>("screen_y", 768);
        preferred_display = config.Get<std::int64_t>("display", 0);
        fullscreen = config.Get<std::int64_t>("full_screen", 0);
        show_borders = config.Get<std::int64_t>("window_borders", 0);
        run_in_background = config.Get<std::int64_t>("run_in_background", 0);

        if (run_in_background) {
            bSoundInBackground = config.Get<std::int64_t>("sound_in_background", 1);
        } else {
            bSoundInBackground = false;
        }

        bSteam = config.Get<std::int64_t>("Steam", 0);
    }

    // initialize SteamApi through evaluating its singleton
    try
    {
        // steamapi::SteamApi::getInstance(!bSteam);
    }
    catch (const std::exception &e)
    {
        spdlog::critical(e.what());
        return EXIT_FAILURE;
    }

    std::shared_ptr<storm::OSWindow> window =
        storm::OSWindow::Create(width, height, preferred_display, fullscreen, show_borders);
    window->SetTitle("Beyond New Horizons");
    window->Subscribe(HandleWindowEvent);
    window->Show();
    core_private->SetWindow(window);

    // Init core
    core_private->InitBase();

    // Message loop
    auto dwOldTime = SDL_GetTicks();

    isRunning = true;
    while (isRunning)
    {
        SDL_Event event{};
        while (SDL_PollEvent(&event) )
        {
            if (auto *editor = core.GetEditor(); editor != nullptr) {
                editor->ProcessEvent(event);
            }
        }

        // if (bActive || run_in_background)
        // {
            if (dwMaxFPS)
            {
                const auto dwMS = 1000u / dwMaxFPS;
                const auto dwNewTime = SDL_GetTicks();
                if (dwNewTime - dwOldTime < dwMS)
                    continue;
                dwOldTime = dwNewTime;
            }

            RunFrameWithOverflowCheck();
        // }
        // else
        // {
            // std::this_thread::sleep_for(std::chrono::milliseconds(50));
        // }

        if (core.Controls && core.Controls->GetDebugAsyncKeyState(VK_F1) && core.Controls->GetDebugAsyncKeyState(VK_SHIFT))
        {
            mi_stats_print_out(mimalloc_fun, nullptr);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    // Release
    core_private->Event("ExitApplication");
    core_private->CleanUp();
    core_private->ReleaseBase();
#ifdef _WIN32 // FIX_LINUX Cursor
    ClipCursor(nullptr);
#elif _DEBUG
    mi_option_set(mi_option_verbose, 0); // disable statistics writing in Linux
#endif
    SDL_Quit();

    return EXIT_SUCCESS;
}
