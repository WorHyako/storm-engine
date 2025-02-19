#include "t_seagulls.h"
#include "entity.h"
#include "core.h"
#include "math_inlines.h"
#include "model.h"
#include "rands.h"
#include "shared/messages.h"

#include "Filesystem/Config/Config.hpp"
#include "Filesystem/Constants/ConfigNames.hpp"

using namespace Storm::Filesystem;

#include <algorithm>

//#pragma warning (disable : 4244)

//--------------------------------------------------------------------
TSeagulls::TSeagulls() : enabled(true), count(0), frightened(false)
{
}

//--------------------------------------------------------------------
TSeagulls::~TSeagulls()
{
    core.EraseEntity(seagullModel);
}

//--------------------------------------------------------------------
void TSeagulls::LoadSettings() {
    auto config = Config::Load(Constants::ConfigNames::animals());
    std::ignore = config.SelectSection(ANIMALS_SEAGULLS_SECTION);
    maxRadius = config.Get<double>("radius", SEAGULL_MAX_RADIUS);
    maxAngleSpeed = config.Get<double>("angle", SEAGULL_MAX_SPEED);
    maxDistance = config.Get<double>("distance", SEAGULL_DISTANCE);
    maxHeight = config.Get<double>("height", SEAGULL_MAX_HEIGHT);
    maxCircleTime = config.Get<std::int64_t>("circle_time", SEAGULL_MAX_CIRCLE_TIME);
    farChoiceChance = config.Get<std::int64_t>("far_choice", SEAGULL_LONG_DISTANCE_CHANCE);
    relaxTime = config.Get<std::int64_t>("relax_time", SEAGULL_RELAX_TIME);
    screamTime = config.Get<std::int64_t>("scream_time", SEAGULL_SCREAM_TIME);
    countAdd = config.Get<std::int64_t>("add_count", SEAGULL_ADD_COUNT);
    std::string screamFilename_str = config.Get<std::string>("scream_file", ANIMALS_SEAGULLS_SCREAM_FILENAME).c_str();
    std::copy(std::begin(screamFilename_str), std::begin(screamFilename_str), screamFilename);
}

//--------------------------------------------------------------------
void TSeagulls::Init()
{
    startY = 0.f;
    LoadSettings();

    renderService = static_cast<VDX9RENDER *>(core.GetService("dx9render"));
    soundService = static_cast<VSoundService *>(core.GetService("SoundService"));

    if (!renderService)
        throw std::runtime_error("!Seagulls: No service: dx9render");
    // if(!soundService)
    //    throw std::runtime_error("!Seagulls: No service: sound");

    seagullModel = core.CreateEntity("MODELR");
    core.Send_Message(seagullModel, "ls", MSG_MODEL_LOAD_GEO, ANIMALS_SEAGULL_FILENAME);
}

//--------------------------------------------------------------------
uint64_t TSeagulls::ProcessMessage(int32_t _code, MESSAGE &message)
{
    const uint32_t outValue = 0;

    switch (_code)
    {
    case MSG_ANIMALS_SEAGULLS_SHOW:
        enabled = true;
        break;

    case MSG_ANIMALS_SEAGULLS_HIDE:
        enabled = false;
        count = 0;
        break;

    case MSG_ANIMALS_SEAGULLS_FRIGHTEN:
        Frighten();
        break;

    case MSG_ANIMALS_SEAGULLS_ADD:
        Add(message.Float(), message.Float(), message.Float());
        break;
    }

    return outValue;
}

//--------------------------------------------------------------------
void TSeagulls::Execute(uint32_t _dTime)
{
    if (!enabled)
        return;

    // <relax>
    if (frightened)
    {
        frightenTime -= _dTime;
        if (frightenTime <= 0)
        {
            frightened = false;
            screamTime <<= 1;
            for (auto i = 0; i < count; i++)
            {
                seagulls[i].va /= 2.0f;
                // seagulls[i].circleTimePassed += _dTime;
            }
        }
    }

    // <all_movements>
    for (auto i = 0; i < count; i++)
    {
        // <scream>
        if (seagulls[i].screamTime > 0)
            seagulls[i].screamTime -= _dTime;
        else
        {
            seagulls[i].screamTime = (rand() % (screamTime >> 1)) + (screamTime >> 1);
            CVECTOR pos(static_cast<float>(seagulls[i].center.x + sin(seagulls[i].a) * seagulls[i].radius),
                        static_cast<float>(seagulls[i].center.z + cos(seagulls[i].a) * seagulls[i].radius),
                        static_cast<float>(seagulls[i].height));

            // if(soundService) soundService->SoundPlay(screamFilename, PCM_3D, VOLUME_FX, false, false, true, 0, &pos);
        }

        // <angle_inc>
        seagulls[i].a = fmodf(seagulls[i].a + (seagulls[i].va / 1000000.0f) * _dTime, 2 * PI);

        // <new_circle>
        if (seagulls[i].circleTimePassed < seagulls[i].circleTime)
            seagulls[i].circleTimePassed += _dTime;
        else
        {
            const auto oldR = seagulls[i].radius;
            seagulls[i].radius = SEAGULL_MIN_RADIUS + rand(maxRadius);
            seagulls[i].circleTimePassed = 0;
            seagulls[i].circleTime = static_cast<int32_t>(rand(static_cast<float>(maxCircleTime)));
            if ((seagulls[i].circleTime) < (maxCircleTime / 20))
                seagulls[i].circleTime = maxCircleTime / 20;
            const auto sinA = sinf(seagulls[i].a);
            const auto cosA = cosf(seagulls[i].a);
            const auto newX1 = seagulls[i].center.x + sinA * (seagulls[i].radius + oldR);
            const auto newZ1 = seagulls[i].center.z + cosA * (seagulls[i].radius + oldR);
            const auto newX2 = seagulls[i].center.x + sinA * (oldR - seagulls[i].radius);
            const auto newZ2 = seagulls[i].center.z + cosA * (oldR - seagulls[i].radius);
            const auto distance1 = fabsf(cameraPos.x - newX1) + fabsf(cameraPos.z - newZ1);
            const auto distance2 = fabsf(cameraPos.x - newX2) + fabsf(cameraPos.z - newZ2);
            auto oldVa = seagulls[i].va;

            seagulls[i].va *= oldR / seagulls[i].radius;
            const auto deltaVa = randCentered(maxAngleSpeed / 5.0f);
            if (((seagulls[i].va + deltaVa) * seagulls[i].va) < 0)
                seagulls[i].va -= deltaVa;
            else
                seagulls[i].va += deltaVa;

            const auto minRadius = maxRadius * maxAngleSpeed / seagulls[i].radius;
            if (fabs(seagulls[i].va) < (minRadius / 2.0f))
            {
                if (seagulls[i].va > 0.0f)
                    seagulls[i].va = minRadius / 2.0f;
                else
                    seagulls[i].va = -minRadius / 2.0f;
            }

            // seagulls[i].height += randCentered(maxHeight / 50.0f);
            if ((distance1 < distance2) && ((rand() % farChoiceChance) == 1))
            {
                seagulls[i].center.x = newX1;
                seagulls[i].center.z = newZ1;
                seagulls[i].va = -seagulls[i].va;
                seagulls[i].a = fmodf(seagulls[i].a + PI, 2 * PI);
            }
            else
            {
                seagulls[i].center.x = newX2;
                seagulls[i].center.z = newZ2;
            }
        }
    }
}

//--------------------------------------------------------------------
void TSeagulls::Realize(uint32_t _dTime)
{
    if (!enabled)
        return;

    float persp;
    renderService->GetCamera(cameraPos, cameraAng, persp);
    if (!count)
        Add(cameraPos.x, cameraPos.y, cameraPos.z);

    auto *seagull = static_cast<MODEL *>(core.GetEntityPointer(seagullModel));
    if (!seagull)
        return;

    for (auto i = 0; i < count; i++)
    {
        CVECTOR ang, pos;
        ang.x = 0.0f;
        ang.z = 0.0f;
        auto angle = seagulls[i].a;
        if (seagulls[i].va > 0.0f)
            ang.y = seagulls[i].a + PI / 2;
        else
            ang.y = seagulls[i].a - PI / 2;
        pos.x = seagulls[i].center.x + sinf(seagulls[i].a) * seagulls[i].radius;
        pos.z = seagulls[i].center.z + cosf(seagulls[i].a) * seagulls[i].radius;
        pos.y = seagulls[i].height;
        seagull->mtx = CMatrix(ang, pos);
        seagull->ProcessStage(Entity::Stage::realize, _dTime);
    }
}

//--------------------------------------------------------------------
void TSeagulls::Frighten()
{
    if (frightened)
        return;

    frightened = true;
    frightenTime = relaxTime;

    for (auto i = 0; i < count; i++)
    {
        seagulls[i].va *= 2.0f;
        seagulls[i].screamTime >>= 2;
    }
    screamTime >>= 1;
}

//--------------------------------------------------------------------
void TSeagulls::Add(float _x, float _y, float _z)
{
    float op;

    for (int i = count; i < (count + countAdd); i++)
    {
        seagulls[i].center = CVECTOR(randCentered(maxDistance) + _x, rand(maxHeight) + SEAGULL_MIN_HEIGHT,
                                     randCentered(maxDistance) + _z);
        seagulls[i].radius = randUpper(maxRadius);
        op = randUpper(maxAngleSpeed);
        if (rand() & 0x1)
            seagulls[i].va = op;
        else
            seagulls[i].va = -op;
        seagulls[i].height = startY + seagulls[i].center.y;
        seagulls[i].a = rand(2 * PI);
        seagulls[i].circleTime = rand() % maxCircleTime;
        seagulls[i].screamTime = (rand() % (screamTime >> 3)) << 3;
    }
    count += countAdd;
}

//--------------------------------------------------------------------
