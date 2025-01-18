// ============================================================================================
// Spirenkov Maxim aka Sp-Max Shaman, 2001
// --------------------------------------------------------------------------------------------
// Storm engine v2.00
// --------------------------------------------------------------------------------------------
// AnimationServiceImp
// --------------------------------------------------------------------------------------------
// Animation service for creating AnimationManager objects
// ============================================================================================

#include "animation_service_imp.h"

#include "core.h"

#include "animation_imp.h"
#include "an_file.h"
#include "string_compare.hpp"

#include "Filesystem/Config/Config.hpp"
#include "Filesystem/Constants/Paths.hpp"

// Unused animation unload time
#define ASRV_DOWNTIME 1
// Longest time span supplied by the AnimationManager
#define ASRV_MAXDLTTIME 50

// ============================================================================================
// Construction, destruction
// ============================================================================================

AnimationServiceImp::AnimationServiceImp()
{
    AnimationImp::SetAnimationService(this);
}

AnimationServiceImp::~AnimationServiceImp()
{
    for (const auto &animation : animations)
    {
        if (animation)
        {
            core.Trace("No release Animation pnt:0x%x for %s.ani", animation, animation->GetAnimationInfo()->GetName());
            delete animation;
        }
    }

    for (const auto &info : ainfo)
        delete info;
}

//============================================================================================

// Phase for running the animation
uint32_t AnimationServiceImp::RunSection()
{
    return SECTION_REALIZE;
};

// Execution functions
void AnimationServiceImp::RunStart()
{
    if (core.Controls->GetDebugAsyncKeyState(VK_F4))
        return;
    auto dltTime = core.GetDeltaTime();
    if (dltTime > 1000)
        dltTime = 1000;
    // Check all animations
    for (int32_t i = 0; i < ainfo.size(); i++)
        if (ainfo[i])
        {
            ainfo[i]->AddDowntime(dltTime);
            if (ainfo[i]->GetDowntime() >= ASRV_DOWNTIME)
            {
                // Unloading unused animation
                // core.Trace("Download animation %s", ainfo[i]->GetName());
                delete ainfo[i];
                ainfo[i] = nullptr;
            }
        }
    // execute all animations
    for (auto i = 0; i < animations.size(); i++)
        if (animations[i])
        {
            int32_t dt;
            for (dt = dltTime; dt > ASRV_MAXDLTTIME; dt -= ASRV_MAXDLTTIME)
                animations[i]->Execute(ASRV_MAXDLTTIME);
            if (dt > 0)
                animations[i]->Execute(dt);
            // core.Trace("Animation: 0x%.8x Time: %f", animation[i], animation[i]->Player(0).GetPosition());
        }
}

void AnimationServiceImp::RunEnd()
{
}

// Create animation for the model, delete using "delete"
Animation *AnimationServiceImp::CreateAnimation(const char *animationName)
{
    // looking for animation, load if not found
    int32_t i;
    for (i = 0; i < ainfo.size(); i++)
        if (ainfo[i])
        {
            if (*ainfo[i] == animationName)
                break;
        }
    if (i == ainfo.size())
    {
        i = LoadAnimation(animationName);
        if (i < 0)
            return nullptr;
    }
    const auto aniIndex = i;
    // The animation is loaded, creating an animation manager
    for (i = 0; i < animations.size(); i++)
        if (!animations[i])
            break;
    if (i == animations.size())
    {
        animations.emplace_back(nullptr);
    }
    animations[i] = new AnimationImp(i, ainfo[aniIndex]);
    return animations[i];
}

// Remove animation (called from destructor)
void AnimationServiceImp::DeleteAnimation(AnimationImp *ani)
{
    Assert(ani);
    Assert(ani->GetThisID() >= 0 || ani->GetThisID() < animations.size());
    Assert(animations[ani->GetThisID()] == ani);
    animations[ani->GetThisID()] = nullptr;
}

// Event
void AnimationServiceImp::Event(const char *eventName)
{
    // Sending a message to the system
    core.Trace("Called function <void AnimationServiceImp::Event(%s)>, please make it.", eventName);
}

// load animation
int32_t AnimationServiceImp::LoadAnimation(const char *animationName)
{
    std::filesystem::path aniPath{Storm::Filesystem::Constants::Paths::animation() / (std::string(animationName) + ".toml")};
    // Open the ini file describing the animation
    auto config = Storm::Filesystem::Config::Load(aniPath);
    std::ignore = config.SelectSection("Main");

    const auto animation_path_opt = config.Get<std::string>("animation");
    if (!animation_path_opt.has_value()) {
        return -1;
    }
    const std::string animation_file = (aniPath.parent_path() / animation_path_opt.value()).string();
    auto *info = new AnimationInfo(animationName);
    if (!LoadAN(animation_file.c_str(), info))
    {
        delete info;
        return -1;
    }

    const auto data_vec = config.Get<std::vector<Storm::Math::Types::Vector2<std::string>>>("data", {{}});
    for (const auto& data : data_vec) {
        auto& userData = info->GetUserData();
        userData[data.x] = data.y;
    }

    const auto sections = config.Sections();
    for (const auto& section : sections) {
        if (section == "Main") {
            continue;
        }
        std::ignore = config.SelectSection(section);
        const auto stime = config.Get<std::int64_t>("start_time", {});
        const auto etime = config.Get<std::int64_t>("end_time", {});

        auto *aci = info->AddAction(section.c_str(), stime, etime);
        if (aci == nullptr)
        {
            core.Trace("Warning! Action [%s] of animation file %s.ani is repeated, skip it", animation_file.c_str(), animationName);
            continue;
        }
        auto rate = config.Get<double>("speed", 1.0);
        aci->SetRate(rate);

        auto type_str = config.Get<std::string>("type", "normal");
        AnimationType type;
        if  (type_str == "normal") {
                type = at_normal;
        } else if (type_str == "reverse") {
                type = at_reverse;
        } else if (type_str == "pingpong") {
                type = at_pingpong;
        } else if (type_str == "rpingpong") {
                type = at_rpingpong;
        }
        aci->SetAnimationType(type);

        const auto ani_loop = config.Get<std::string>("loop", "false");
        bool is_loop = false;
        if (ani_loop == "false") {
            is_loop = false;
        } else if (ani_loop == "true") {
            is_loop = true;
        }
        aci->SetLoop(is_loop);

        const auto event_vec = config.Get<std::vector<Storm::Math::Types::Vector3<std::string>>>("event", {});
        for (auto& event : event_vec) {
            ExtAnimationEventType event_type = eae_normal;
            if (event.z == "always")
            {
                event_type = eae_always;
            }
            else if (event.z == "normal")
            {
                event_type = eae_normal;
            }
            else if (event.z == "reverse")
            {
                event_type = eae_reverse;
            }
            std::int64_t event_time = std::stoi(event.y);
            event_time = std::clamp(event_time, stime, etime);

            aci->AddEvent(event.x.c_str(), event_time, event_type);
        }
        const auto ani_data_vec = config.Get<std::vector<Storm::Math::Types::Vector2<std::string>>>("data", {});
        for (const auto& data : ani_data_vec) {
            auto& aciData = aci->GetUserData();
            aciData[data.x] = data.y;
        }
    }

    // Looking for a free pointer
    int32_t i;
    for (i = 0; i < ainfo.size(); i++) {
        if (ainfo[i] == nullptr) {
            break;
        }
    }
    // expand the array if not found
    if (i == ainfo.size()) {
        ainfo.emplace_back(nullptr);
    }
    ainfo[i] = info;
    return i;
}

// load AN
bool AnimationServiceImp::LoadAN(const char *fname, AnimationInfo *info)
{
    std::fstream fileS;
    try
    {
        fileS = fio->_CreateFile(fname, std::ios::binary | std::ios::in);
        if (!fileS.is_open())
        {
            core.Trace("Cannot open file: %s", fname);
            return false;
        }
        // Reading the file header
        ANFILE::HEADER header;
        if (!fio->_ReadFile(fileS, &header, sizeof(ANFILE::HEADER)) || header.nFrames <= 0 || header.nJoints <= 0 ||
            header.framesPerSec < 0.0f || header.framesPerSec > 1000.0f)
        {
            core.Trace("Incorrect file header in animation file: %s", fname);
            fio->_CloseFile(fileS);
            return false;
        }
        // Set animation time
        info->SetNumFrames(header.nFrames);
        // Set the animation speed
        info->SetFPS(header.framesPerSec);
        // Create the required number of bones
        info->CreateBones(header.nJoints);
        // Setting parents
        auto *const prntIndeces = new int32_t[header.nJoints];
        if (!fio->_ReadFile(fileS, prntIndeces, header.nJoints * sizeof(int32_t)))
        {
            core.Trace("Incorrect parent indeces block in animation file: %s", fname);
            delete[] prntIndeces;
            fio->_CloseFile(fileS);
            return false;
        }
        for (int32_t i = 1; i < header.nJoints; i++)
        {
            Assert(prntIndeces[i] >= 0 || prntIndeces[i] < header.nJoints);
            Assert(prntIndeces[i] != i);
            info->GetBone(i).SetParent(&info->GetBone(prntIndeces[i]));
        }
        delete[] prntIndeces;
        // Starting positions of bones
        auto *vrt = new CVECTOR[header.nJoints];
        if (!fio->_ReadFile(fileS, vrt, header.nJoints * sizeof(CVECTOR)))
        {
            core.Trace("Incorrect start joints position block block in animation file: %s", fname);
            delete[] vrt;
            fio->_CloseFile(fileS);
            return false;
        }
        for (int32_t i = 0; i < header.nJoints; i++)
        {
            info->GetBone(i).SetNumFrames(header.nFrames, vrt[i], i == 0);
        }
        delete[] vrt;

        // Root bone positions
        vrt = new CVECTOR[header.nFrames];
        if (!fio->_ReadFile(fileS, vrt, header.nFrames * sizeof(CVECTOR)))
        {
            core.Trace("Incorrect root joint position block block in animation file: %s", fname);
            delete[] vrt;
            fio->_CloseFile(fileS);
            return false;
        }
        info->GetBone(0).SetPositions(vrt, header.nFrames);
        delete[] vrt;

        // Angles
        auto *ang = new Quaternion[header.nFrames];
        for (int32_t i = 0; i < header.nJoints; i++)
        {
            if (!fio->_ReadFile(fileS, ang, header.nFrames * sizeof(*ang)))
            {
                core.Trace("Incorrect joint angle block (%i) block in animation file: %s", i, fname);
                fio->_CloseFile(fileS);
                return false;
            }
            info->GetBone(i).SetAngles(ang, header.nFrames);
        }
        delete[] ang;

        //-----------------------------------------------
        for (int32_t i = 0; i < header.nJoints; i++)
        {
            info->GetBone(i).BuildStartMatrix();
        }
        for (int32_t i = 0; i < header.nJoints; i++)
        {
            info->GetBone(i).start.Transposition();
        }
        //-----------------------------------------------

        // Close the file
        fio->_CloseFile(fileS);
        return true;
    }
    catch (...)
    {
        if (fileS.is_open())
        {
            fio->_CloseFile(fileS);
        }
        core.Trace("Error reading animation file: %s", fname);
        return false;
    }
}
