I will destroy the build scripts of this project and create them again.

Each character of the CMake files will be incinerated.

Every CMake file will be reborn from the hell of the past.

Every thought of the previous author will be forgotten.

I'm organizing a "Night Parade of One Hundred Demons"* as his worshipper, as his WorHyako.

\* Hyakki Yagyō (百鬼夜行, "Night Parade of One Hundred Demons")

---

## Progress table for source files

**\*\<modules\> = storm-engine/src/modules**

**\*\<libs\> = storm-engine/src/libs**

|       Module name       |       State        |        Old folder         |     New folder      |                                                            Old dependencies                                                             | New dependencies |
|:-----------------------:|:------------------:|:-------------------------:|:-------------------:|:---------------------------------------------------------------------------------------------------------------------------------------:|:----------------:|
|     storm::animals      |         -          |     \<libs\>/animals      |                     |                  animation<br/>collide<br/>core<br/>geometry<br/>model<br/>renderer<br/>sea<br/>ship<br/>sound_service                  |                  |
|    storm::animation     |         -          |    \<libs\>/animation     |                     |                                                              core<br/>util                                                              |                  |
|   storm::ball_splash    |         -          |   \<libs\>/ball_splash    |                     |                                                 core<br/>geometry<br/>renderer<br/>sea                                                  |                  |
| storm::battle_interface |         -          | \<libs\>/battle_interface |                     |             animation<br/>core<br/>geometry<br/>island<br/>model<br/>particles<br/>renderer<br/>sea_ai<br/>ship<br/>weather             |                  |
|      storm::blade       |         -          |      \<libs\>/blade       |                     |                                   animation<br/>collide<br/>core<br/>geometry<br/>model<br/>renderer                                    |                  |
|       storm::blot       |         -          |       \<libs\>/blot       |                     |                                                core<br/>geometry<br/>model<br/>renderer                                                 |                  |
|     storm::collide      |         -          |     \<libs\>/collide      |                     |                                                                  core                                                                   |                  |
|      storm::config      |         -          |      \<libs\>/config      |                     |                                                                  core                                                                   |                  |
|       storm::core       |         -          |       \<libs\>/core       |                     | diagnostics<br/>editor<br/>math<br/>shared_headers<br/>steam_apia<br/>fast_float<br/>SDL2<br/>window<br/>tomlplusplus<br/>nlohmann_json |                  |
|   storm::diagnostics    |         -          |   \<libs\>/diagnostics    |                     |                                               core<br/>util<br/>spdlog<br/>sentry-native                                                |                  |
|      storm::dialog      |         -          |      \<libs\>/dialog      |                     |                                       core<br/>geometry<br/>model<br/>renderer<br/>sound_service                                        |                  |
|      storm::editor      |         -          |      \<libs\>/editor      |                     |                                                         core<br/>imgui_backend                                                          |                  |
|     storm::geometry     |         -          |     \<libs\>/geometry     |                     |                                                            core<br/>renderer                                                            |                  |
|      storm::input       | :white_check_mark: |      \<libs\>/input       |  \<modules\>/input  |                                                          utils<br/>SDL2 (Win)                                                           |   storm::utils   |
|      storm::island      |         -          |      \<libs\>/island      |                     |                           collide<br/>core<br/>geometry<br/>model<br/>renderer<br/>sea<br/>sea_ai<br/>weather                           |                  |
|     storm::lighter      |         -          |     \<libs\>/lighter      |                     |                                            core<br/>geometry<br/>model<br/>renderer<br/>util                                            |                  |
|     storm::location     |         -          |     \<libs\>/location     |                     |             animation<br/>blade<br/>collide<br/>core<br/>geometry<br/>model<br/>renderer<br/>sea<br/>sound_service<br/>util             |                  |
|     storm::locator      |         -          |     \<libs\>/locator      |                     |                                                      core<br/>renderer<br/>sea_ai                                                       |                  |
|       storm::mast       |         -          |       \<libs\>/mast       |                     |                                  collide<br/>core<br/>geometry<br/>island model<br/>renderer<br/>ship                                   |                  |
|       storm::math       |         +          |       \<libs\>/math       |  \<modules\>/math   |                                                                                                                                         |                  |
|      storm::model       |         -          |      \<libs\>/model       |                     |                                        animation<br/>collide<br/>core<br/>geometry<br/>renderer                                         |                  |
|    storm::particles     |         -          |    \<libs\>/particles     |                     |                                                 core<br/>geometry<br/>renderer<br/>util                                                 |                  |
|   storm::pcs_controls   |         -          |   \<libs\>/pcs_controls   |                     |                                                             core<br/>input                                                              |                  |
|     storm::renderer     |         -          |     \<libs\>/renderer     |                     |                  core<br/>config<br/>directx<br/>util<br/>"legacy_stdio_definitions"(Win)<br/>NATIVE_D3D9_LIBS(Linux)                   |                  |
|     storm::rigging      |         -          |     \<libs\>/rigging      |                     |                                core<br/>geometry<br/>model<br/>renderer<br/>sea_ai<br/>ship<br/>weather                                 |                  |
|     storm::sailors      |         -          |     \<libs\>/sailors      |                     |                           animation<br/>collide<br/>core<br/>geometry<br/>model<br/>renderer<br/>sea<br/>ship                           |                  |
|  storm::script_library  |         -          |  \<libs\>/script_library  |                     |                                                                  core                                                                   |                  |
|       storm::sea        |         -          |       \<libs\>/sea        |                     |                                                      core<br/>renderer<br/>sea_ai                                                       |                  |
|      storm::sea_ai      |         -          |      \<libs\>/sea_ai      |                     |               collide<br/>core<br/>geometry<br/>island<br/>location<br/>model<br/>particles<br/>renderer<br/>sea<br/>ship               |                  |
|   storm::sea_cameras    |         -          |   \<libs\>/sea_cameras    |                     |                           collide<br/>core<br/>geometry<br/>island<br/>model<br/>renderer<br/>sea<br/>sea_ai                            |                  |
|  storm::sea_creatures   |         -          |  \<libs\>/sea_creatures   |                     |                                animation<br/>core<br/>geometry<br/>island<br/>renderer<br/>sea<br/>ship                                 |                  |
|     storm::sea_foam     |         -          |     \<libs\>/sea_foam     |                     |                  core<br/>geometry<br/>model<br/>particles<br/>renderer<br/>sea<br/>sea_ai<br/>ship<br/>sound_service                   |                  |
|   storm::sea_operator   |         -          |   \<libs\>/sea_operator   |                     |                                             core<br/>geometry<br/>renderer<br/>sea<br/>ship                                             |                  |
|      storm::shadow      |         -          |      \<libs\>/shadow      |                     |                                          collide<br/>core<br/>geometry<br/>model<br/>renderer                                           |                  |
|  storm::shared_headers  |         -          |  \<libs\>/shared_header   |                     |                                                                  core                                                                   |                  |
|       storm::ship       |         -          |       \<libs\>/ship       |                     |              collide<br/>core<br/>geometry<br/>island<br/>location<br/>model<br/>sea<br/>sea_ai<br/>particles<br/>renderer              |                  |
|   storm::sink_effect    |         -          |   \<libs\>/sink_effect    |                     |                                     core<br/>geometry<br/>model<br/>sea<br/>ship<br/>sound_service                                      |                  |
|      storm::sound       |         -          |      \<libs\>/sound       |                     |                                                   core<br/>renderer<br/>sound_service                                                   |                  |
|  storm::sound_service   |         -          |  \<libs\>/sound_service   |                     |                                                       core<br/>fmod<br/>renderer                                                        |                  |
|    storm::steam_api     |         -          |    \<libs\>/steam_api     |                     |                                                       core<br/>steamworks(Steam)                                                        |                  |
|     storm::teleport     |         -          |     \<libs\>/teleport     |                     |                                                   core<br/>pcs_controls<br/>renderer                                                    |                  |
|     storm::tornado      |         -          |     \<libs\>/tornado      |                     |                               core<br/>geometry<br/>model<br/>renderer<br/>sea<br/>ship<br/>sound_service                               |                  |
|      storm::touch       |         -          |      \<libs\>/touch       |                     |                                     core<br/>geometry<br/>island<br/>location<br/>renderer<br/>ship                                     |                  |
|      storm::utils       |         +          |      \<libs\>/utils       |  \<modules\>/utils  |                                                               SDL2 (Win)                                                                |    SDL2 (Win)    |
|   storm::water_rings    |         -          |   \<libs\>/water_rings    |                     |                                      collide<br/>geometry<br/>core<br/>model<br/>renderer<br/>sea                                       |                  |
|     storm::weather      |         -          |          weather          |                     |                                       collide<br/>core<br/>geometry<br/>renderer<br/>sea<br/>ship                                       |                  |
|     storm::windows      |         +          |      \<libs\>/window      | \<modules\>/windows |                                                                  SDL2                                                                   |       SDL2       |
|    storm::world_map     |         -          |         world_map         |                     |                                battle_interface<br/>core<br/>geometry<br/>location<br/>renderer<br/>util                                |                  |
|    storm::xinterface    |         -          |        xinterface         |                     |                    animation<br/>core<br/>geometry<br/>model<br/>renderer<br/>util<br/>ddraw(Win)<br/>amstrmid(Win)                     |                  |

## Progress table for ThirdParty libs

| Module name | State | Folder | Dependencies |
|:-----------:|:-----:|:------:|:------------:|
|             |       |        |              |
|             |       |        |              |
|             |       |        |              |
|             |       |        |              |
|             |       |        |              |
|             |       |        |              |
|             |       |        |              |