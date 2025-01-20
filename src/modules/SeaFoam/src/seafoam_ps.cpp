#include "seafoam_ps.h"

#include "core.h"

#include "entity.h"
#include "object.h"

#include "Filesystem/Config/Config.hpp"
#include "Filesystem/Constants/ConfigNames.hpp"

using namespace Storm::Filesystem;

SEAFOAM_PS::SEAFOAM_PS() : enableEmit(true)
{
    ParticleColor = 0xffffffff;

    bTrackAngle = false;

    l_PTR = nullptr;
    r_PTR = nullptr;

    bLinkEmitter = false;

    RenderService = nullptr;
    ParticlesNum = 0;
    TexturesNum = 0;
    Particle = nullptr;
    VBuffer = nullptr;

    Emitter.x = Emitter.y = Emitter.z = 0;
    Camera_EmitterPos.x = Camera_EmitterPos.y = Camera_EmitterPos.z = 0;
    EmitterDirection.x = EmitterDirection.y = EmitterDirection.z = 0;
    DirectionDeviation = 0.0f;

    bColorInverse = false;
    bUniformEmit = false;
    bRandomDirection = false;
    bRepeat = false;
    bComplete = false;

    ESpace = 0;

    EmitIndex = 0;
    EmitTimeDelta = 0;
    Delay = 0;

    DeltaTimeSLE = 0;
    nEmitted = 0;
    nSystemLifeTime = 0;

    vWindDirection.x = 0;
    vWindDirection.y = 0;
    vWindDirection.z = 0.0;
    fWindPower = 0;
    fWindEffect = 0;

    pFlowTrack = nullptr;
    nFlowTrackSize = 0;
    bUseFlowTrack = false;
    bLayOnSurface = false;
}

SEAFOAM_PS::~SEAFOAM_PS()
{
    int32_t n;
    if (VBuffer)
        VBuffer->Release();
    if (RenderService)
    {
        for (n = 0; n < TexturesNum; n++)
            RenderService->TextureRelease(TextureID[n]);
        // core.FreeService("dx9render");
    }
    delete Particle;
    Particle = nullptr;
    delete pFlowTrack;
    pFlowTrack = nullptr;
}

// node ----------------------------------------------------------------------------
SEAFOAM_PS *SEAFOAM_PS::GetLeftNode()
{
    return l_PTR;
}

SEAFOAM_PS *SEAFOAM_PS::GetRightNode()
{
    return r_PTR;
}

void SEAFOAM_PS::SetLeftNode(SEAFOAM_PS *node)
{
    l_PTR = node;
}

void SEAFOAM_PS::SetRightNode(SEAFOAM_PS *node)
{
    r_PTR = node;
}

void SEAFOAM_PS::Attach(SEAFOAM_PS **Root, SEAFOAM_PS **Top)
{
    if (*Root == nullptr)
    {
        *Root = this;
        *Top = this;
        return;
    }
    (*Top)->SetRightNode(this);
    SetLeftNode(*Top);
    *Top = this;
}

void SEAFOAM_PS::AttachTo(SEAFOAM_PS *la_PTR, SEAFOAM_PS **Root, SEAFOAM_PS **Top)
{
    SEAFOAM_PS *t_PTR;
    t_PTR = la_PTR->GetRightNode();
    la_PTR->SetRightNode(this);
    SetRightNode(t_PTR);
    SetLeftNode(la_PTR);
    if (t_PTR)
    {
        t_PTR->SetLeftNode(this);
    }
    else
        *Top = this;
}

void SEAFOAM_PS::Deattach(SEAFOAM_PS **Root, SEAFOAM_PS **Top)
{
    if (l_PTR)
        l_PTR->SetRightNode(r_PTR);
    if (r_PTR)
        r_PTR->SetLeftNode(l_PTR);
    if (*Root == this)
        *Root = r_PTR;
    if (*Top == this)
        *Top = l_PTR;
}

void SEAFOAM_PS::ProcessOrder(SEAFOAM_PS **Root, SEAFOAM_PS **Top)
{
    if (r_PTR)
    {
        // if(Camera_EmitterPos.z > r_PTR->Camera_EmitterPos.z)
        if (Camera_EmitterPosA.z < r_PTR->Camera_EmitterPosA.z)
        {
            Deattach(Root, Top);
            AttachTo(r_PTR, Root, Top);
        }
    }
}

//----------------------------------------------------------------------------------

bool SEAFOAM_PS::Init(const std::string_view& section)
{
    // GUARD(SEAFOAM_PS::Init)
    // load render service -----------------------------------------------------
    RenderService = static_cast<VDX9RENDER *>(core.GetService("dx9render"));
    if (!RenderService)
        throw std::runtime_error("No service: dx9render");

    gs = static_cast<VGEOMETRY *>(core.GetService("geometry"));
    // if(!gs) return false;


    auto config = Config::Load(Constants::ConfigNames::particles());
    std::ignore = config.SelectSection(std::string(section));

    TexturesNum = 0;
    auto texture_name_opt = config.Get<std::vector<std::string>>(PSKEY_TEXTURE);

    if (texture_name_opt.has_value()) {
        for (int i = 0; i < std::size(texture_name_opt.value()); i++) {
            TextureID[i] = RenderService->TextureCreate(texture_name_opt.value()[i].c_str());
            if (TextureID[i] >= 0)
                TexturesNum++;
        }
    }

    TechniqueName = config.Get<std::string>(PSKEY_TECHNIQUE, {});

    // configure particles
    ParticlesNum = config.Get<std::int64_t>(PSKEY_PNUM, 32);
    EmissionTime = config.Get<double>(PSKEY_EMISSIONTIME, 0.0);
    DeltaTimeSLE = static_cast<std::int32_t>(EmissionTime);
    EmissionTimeRand = config.Get<double>(PSKEY_EMISSIONTIMERAND, 0.0);
    CurrentEmissionTimeRand = EmissionTimeRand * rand() / RAND_MAX;
    fSurfaceOffset = config.Get<double>(PSKEY_SURFACEOFFSET, 0);
    ParticleColor = config.Get<std::int64_t>("color", 0xffffffff);

    fWindEffect = config.Get<double>(PSKEY_WINDEFFECT, 0.0);

    DirectionDeviation = config.Get<double>(PSKEY_DDEVIATION, 0.0);
    Gravity = config.Get<double>(PSKEY_GRAVITY, 0.0);
    Inispeed = config.Get<double>(PSKEY_INISPEED, 0.0);
    SpeedDeviation = config.Get<double>(PSKEY_SDEVIATION, 0.0);
    Lifetime = config.Get<std::int64_t>(PSKEY_LIFETIME, 1000);
    Spin = config.Get<double>(PSKEY_SPIN, 0.0);
    SpinDeviation = config.Get<double>(PSKEY_SPINDEV, 0.0);
    EmitterIniTime = config.Get<std::int64_t>(PSKEY_EMITTERINITIME, 0);
    Weight = config.Get<double>(PSKEY_WEIGHT, 0.0);
    WeightDeviation = config.Get<double>(PSKEY_WEIGHTDEVIATION, 0.0);
    Emitdelta = config.Get<std::int64_t>(PSKEY_EMITDELTA, 0);
    ESpace = config.Get<double>(PSKEY_EMITRADIUS, 0.0);
    fTrackPointRadius = config.Get<double>(PSKEY_TRACKPOINTRADIUS, 1.0);

    bColorInverse = config.Get<std::int64_t>(PSKEY_COLORINVERSE, 0);
    bUniformEmit = config.Get<std::int64_t>(PSKEY_UNIFORMEMIT, 0);
    bRandomDirection = config.Get<std::int64_t>(PSKEY_RANDOMDIRECTION, 0);
    bRepeat = config.Get<std::int64_t>(PSKEY_NONSTOPEMIT, 0);

    Particle = (PARTICLE *)new char[ParticlesNum * sizeof(PARTICLE)];
    if (Particle == nullptr)
        throw std::runtime_error("mem error");

    memset(Particle, 0, ParticlesNum * sizeof(PARTICLE));

    for (int n = 0; n < ParticlesNum; n++)
    {
        Particle[n].pos.x = Emitter.x + ESpace * (0.5f - static_cast<float>(rand()) / RAND_MAX);
        Particle[n].pos.y = Emitter.y + ESpace * (0.5f - static_cast<float>(rand()) / RAND_MAX);
        Particle[n].pos.z = Emitter.z + ESpace * (0.5f - static_cast<float>(rand()) / RAND_MAX);
        // Particle[n].size = 1.0f;
        Particle[n].size = 0.0f;
        Particle[n].color = 0xffffffff;
        Particle[n].color = ParticleColor;

        Particle[n].weight = Weight + WeightDeviation * (0.5f - static_cast<float>(rand()) / RAND_MAX);

        Particle[n].speedVal = Inispeed + SpeedDeviation * (0.5f - static_cast<float>(rand()) / RAND_MAX);
        Particle[n].speed = 0; // Particle[n].speedVal;

        if (bRandomDirection)
        {
            Particle[n].ang.x = (0.5f - static_cast<float>(rand()) / RAND_MAX);
            Particle[n].ang.y = (0.5f - static_cast<float>(rand()) / RAND_MAX);
            Particle[n].ang.z = (0.5f - static_cast<float>(rand()) / RAND_MAX);
        }
        else
        {
            Particle[n].ang.x =
                EmitterDirection.x + DirectionDeviation * (0.5f - static_cast<float>(rand()) / RAND_MAX);
            Particle[n].ang.y =
                EmitterDirection.y + DirectionDeviation * (0.5f - static_cast<float>(rand()) / RAND_MAX);
            Particle[n].ang.z =
                EmitterDirection.z + DirectionDeviation * (0.5f - static_cast<float>(rand()) / RAND_MAX);
        }
        Particle[n].ang = !Particle[n].ang;

        float ChaosVal = 0.0001f;
        Particle[n].chaos.x = ChaosVal * (0.5f - static_cast<float>(rand()) / RAND_MAX);
        Particle[n].chaos.y = ChaosVal * (0.5f - static_cast<float>(rand()) / RAND_MAX);
        Particle[n].chaos.z = ChaosVal * (0.5f - static_cast<float>(rand()) / RAND_MAX);
        Particle[n].speed_chaos = 1.0f - 0.1f * (static_cast<float>(rand()) / RAND_MAX);

        Particle[n].v = Particle[n].ang * Particle[n].speed;
        Particle[n].lifetime = Lifetime;

        Particle[n].time = 0;
        Particle[n].live = false;
        Particle[n].done = false;
        // if(bUniformEmit) Particle[n].time = -n*(Lifetime/ParticlesNum);
        // else Particle[n].time = -EmitterIniTime * rand()/RAND_MAX;

        Particle[n].spinVal = Spin + SpinDeviation * (0.5f - static_cast<float>(rand()) / RAND_MAX);
        Particle[n].spin = Particle[n].spinVal;
    }

    // build tracks
    BuildTrack(Visibility, section, PSKEY_ALPHAKEY);
    BuildTrack(ParticleSize, section, PSKEY_PSIZEKEY);
    BuildTrack(ParticleSpeed, section, PSKEY_PSPEEDKEY);
    BuildTrack(ParticleSpin, section, PSKEY_PSPINKEY);
    BuildTrack(WindEffect, section, PSKEY_WINDEFFECTKEY);
    if (BuildTrack(ParticleAngle, section, PSKEY_PANGLEKEY))
        bTrackAngle = true;
    else
        bTrackAngle = false;

    // create vertex buffer
    RenderService->CreateVertexBuffer(sizeof(PARTICLE_VERTEX) * VERTEXS_ON_PARTICLE * ParticlesNum,
                                      D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, PARTICLE_FVF, D3DPOOL_SYSTEMMEM, &VBuffer);
    if (VBuffer == nullptr)
        throw std::runtime_error("vbuffer error");

    UpdateVertexBuffer();

    // UNGUARD
    return true;
}

void SEAFOAM_PS::UpdateVertexBuffer()
{
    CVECTOR ipos[4];
    CVECTOR rpos[4];
    CVECTOR pos;
    CVECTOR local_pos;
    PARTICLE_VERTEX *pVertex;
    int32_t n, i;
    int32_t index;
    float halfsize;
    CMatrix RMatrix;

    Camera_EmitterPosA.x = Camera_EmitterPosA.y = Camera_EmitterPosA.z = 0;

    RenderService->VBLock(VBuffer, 0, sizeof(PARTICLE_VERTEX) * VERTEXS_ON_PARTICLE * ParticlesNum,
                          (uint8_t **)&pVertex, 0);
    for (n = 0; n < ParticlesNum; n++)
    {
        index = n * VERTEXS_ON_PARTICLE;

        // RenderService->GetTransform(D3DTS_VIEW,Matrix); set for lock particles in screen zero axis
        local_pos = Matrix * Particle[n].pos;

        Camera_EmitterPosA += local_pos;

        halfsize = Particle[n].size / 2.0f;

        ipos[0].x = -halfsize;
        ipos[0].y = halfsize;
        ipos[0].z = 0;

        ipos[1].x = -halfsize;
        ipos[1].y = -halfsize;
        ipos[1].z = 0;

        ipos[2].x = halfsize;
        ipos[2].y = -halfsize;
        ipos[2].z = 0;

        ipos[3].x = halfsize;
        ipos[3].y = halfsize;
        ipos[3].z = 0;

        RMatrix.BuildRotateZ(Particle[n].angle);

        rpos[0] = RMatrix * ipos[0];
        rpos[1] = RMatrix * ipos[1];
        rpos[2] = RMatrix * ipos[2];
        rpos[3] = RMatrix * ipos[3];

        /*rpos[0] = ipos[0];
        rpos[1] = ipos[1];
        rpos[2] = ipos[2];
        rpos[3] = ipos[3];*/

        // first & second left up
        pos.x = local_pos.x + rpos[0].x; // - halfsize;
        pos.y = local_pos.y + rpos[0].y; // halfsize;
        pos.z = local_pos.z;
        pVertex[index].pos = pos;
        pVertex[index + 3].pos = pos;

        pVertex[index].tu = 0.0f;
        pVertex[index].tv = 0.0f;
        pVertex[index + 3].tu = 0.0f;
        pVertex[index + 3].tv = 0.0f;

        // first left down
        pos.x = local_pos.x + rpos[1].x; // -halfsize;
        pos.y = local_pos.y + rpos[1].y; // -halfsize;
        pos.z = local_pos.z;
        pVertex[index + 1].pos = pos;

        pVertex[index + 1].tu = 0.0f;
        pVertex[index + 1].tv = 1.0f;

        // first & second right down
        pos.x = local_pos.x + rpos[2].x; // halfsize;
        pos.y = local_pos.y + rpos[2].y; //-halfsize;
        pos.z = local_pos.z;
        pVertex[index + 2].pos = pos;
        pVertex[index + 4].pos = pos;

        pVertex[index + 2].tu = 1.0f;
        pVertex[index + 2].tv = 1.0f;
        pVertex[index + 4].tu = 1.0f;
        pVertex[index + 4].tv = 1.0f;

        // second right up
        pos.x = local_pos.x + rpos[3].x; // halfsize;
        pos.y = local_pos.y + rpos[3].y; // halfsize;
        pos.z = local_pos.z;
        pVertex[index + 5].pos = pos;

        pVertex[index + 5].tu = 1.0f;
        pVertex[index + 5].tv = 0.0f;

        for (i = index; i < (index + VERTEXS_ON_PARTICLE); i++)
        {
            pVertex[i].color = Particle[n].color;
        }
    }
    RenderService->VBUnlock(VBuffer);
    if (ParticlesNum)
    {
        Camera_EmitterPosA.x = Camera_EmitterPosA.x / ParticlesNum;
        Camera_EmitterPosA.y = Camera_EmitterPosA.y / ParticlesNum;
        Camera_EmitterPosA.z = Camera_EmitterPosA.z / ParticlesNum;
    }
}

void SEAFOAM_PS::Execute(uint32_t DeltaTime)
{
    /*if(Delay > 0) {    Delay = Delay - DeltaTime;    return;    }

    if(bLinkEmitter)
    {
      COLLISION_OBJECT * pLink;
      pLink = (COLLISION_OBJECT *)core.GetEntityPointer(LinkObject);
      if(pLink)
      {
        Emitter = pLink->mtx * LinkPos;
        EmitterDirection = pLink->mtx * LinkDirPos;
        EmitterDirection = EmitterDirection - Emitter;
        EmitterDirection = !EmitterDirection;
      }
    }
    ProcessParticles(DeltaTime);
    SetParticlesTracks(DeltaTime);
    UpdateVertexBuffer();*/
}

void SEAFOAM_PS::LayOnSurface(uint32_t index)
{
    COLLISION_OBJECT *pLink;
    CVECTOR from, to;
    float dist;
    pLink = static_cast<COLLISION_OBJECT *>(core.GetEntityPointer(SurfaceID));
    if (pLink == nullptr)
        return;
    from = Particle[index].pos;
    to = from;
    from.y = 100.0f;
    to.y = -100.0f;
    dist = pLink->Trace(from, to);
    Particle[index].pos.y = from.y + dist * (to.y - from.y) + fSurfaceOffset;
}

void SEAFOAM_PS::Realize(uint32_t DeltaTime)
{
    if (Delay > 0)
    {
        Delay = Delay - DeltaTime;
        return;
    }

    if (bLinkEmitter)
    {
        COLLISION_OBJECT *pLink;
        pLink = static_cast<COLLISION_OBJECT *>(core.GetEntityPointer(LinkObject));
        if (pLink)
        {
            Emitter = pLink->mtx * LinkPos;
            EmitterDirection = pLink->mtx * LinkDirPos;
            EmitterDirection = EmitterDirection - Emitter;
            EmitterDirection = !EmitterDirection;
        }
    }

    RenderService->GetTransform(D3DTS_VIEW, Matrix);

    // Camera_EmitterPos = Matrix * Emitter;

    RenderService->GetCamera(CameraPos, CameraAng, Perspective);

    const CMatrix IMatrix;
    RenderService->SetTransform(D3DTS_VIEW, IMatrix);
    RenderService->SetTransform(D3DTS_WORLD, IMatrix);
    ProcessParticles(DeltaTime);
    SetParticlesTracks(DeltaTime);
    UpdateVertexBuffer();

    RenderService->TextureSet(0, TextureID[0]);

    RenderService->SetFVF(PARTICLE_FVF);
    RenderService->SetStreamSource(0, VBuffer, sizeof(PARTICLE_VERTEX));
    // RenderService->SetIndices(0, 0);

    bool bDraw;
    // if(bColorInverse)bDraw = RenderService->TechniqueExecuteStart("particles_inv");
    // else bDraw = RenderService->TechniqueExecuteStart("particles");

    bDraw = RenderService->TechniqueExecuteStart(TechniqueName.c_str());
    if (bDraw)
    {
        RenderService->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2 * ParticlesNum);
        while (RenderService->TechniqueExecuteNext())
        {
        }
    }

    RenderService->SetTransform(D3DTS_VIEW, Matrix);
}

bool SEAFOAM_PS::EmitParticle()
{
    if (!enableEmit)
        return false;

    int32_t n;
    for (n = 0; n < ParticlesNum; n++)
    {
        if (Particle[n].live)
            continue; // search dead particle
        if (Particle[n].done && (!bRepeat))
            continue;
        // emit new

        Particle[n].time = 0;
        Particle[n].flow_track_index = 0;
        Particle[n].pos.x = Emitter.x + ESpace * (0.5f - static_cast<float>(rand()) / RAND_MAX);
        Particle[n].pos.y = Emitter.y + ESpace * (0.5f - static_cast<float>(rand()) / RAND_MAX);
        Particle[n].pos.z = Emitter.z + ESpace * (0.5f - static_cast<float>(rand()) / RAND_MAX);

        Particle[n].speed = 0; // Inispeed + SpeedDeviation*(0.5f - (float)rand()/RAND_MAX);

        if (bRandomDirection)
        {
            Particle[n].ang.x = (0.5f - static_cast<float>(rand()) / RAND_MAX);
            Particle[n].ang.y = (0.5f - static_cast<float>(rand()) / RAND_MAX);
            Particle[n].ang.z = (0.5f - static_cast<float>(rand()) / RAND_MAX);
        }
        else
        {
            Particle[n].ang.x =
                EmitterDirection.x + DirectionDeviation * (0.5f - static_cast<float>(rand()) / RAND_MAX);
            Particle[n].ang.y =
                EmitterDirection.y + DirectionDeviation * (0.5f - static_cast<float>(rand()) / RAND_MAX);
            Particle[n].ang.z =
                EmitterDirection.z + DirectionDeviation * (0.5f - static_cast<float>(rand()) / RAND_MAX);
        }
        Particle[n].ang = !Particle[n].ang;
        Particle[n].v = Particle[n].ang * Particle[n].speed;
        Particle[n].live = true;
        if (bLayOnSurface)
            LayOnSurface(n);

        return true;
    }
    return false;
}

void SEAFOAM_PS::ProcessParticles(uint32_t DeltaTime)
{
    int32_t n;

    if (nSystemLifeTime > 0)
    {
        nSystemLifeTime = nSystemLifeTime - DeltaTime;
        if (nSystemLifeTime <= 0)
            bRepeat = false;
    }

    for (n = 0; n < ParticlesNum; n++)
    {
        if (!Particle[n].live)
            continue;

        Particle[n].time += DeltaTime;
        if (Particle[n].time > Lifetime)
        {
            // particle done
            Particle[n].live = false;
            if (!bRepeat)
                Particle[n].done = true;
            Particle[n].size = 0;
            Particle[n].color = 0;
            Particle[n].color = 0xffffff;
            Particle[n].color = ParticleColor;
            if (ParticleColor != 0xffffffff)
            {
                Particle[n].color &= 0xff000000;
                Particle[n].color |= 0xffffff * rand() / RAND_MAX;
            }

            Particle[n].time = 0;
            continue;
        }

        // update particle parameters
        if (bUseFlowTrack)
        {
            Particle[n].pos.x += DeltaTime * (Particle[n].v.x + Particle[n].chaos.x);
            Particle[n].pos.y += DeltaTime * (Particle[n].v.y + Particle[n].chaos.y);
            Particle[n].pos.z += DeltaTime * (Particle[n].v.z + Particle[n].chaos.z);

            // Particle[n].angle += DeltaTime * Particle[n].spin;
            SetFlowTrack(n);

            // Particle[n].pos.y -= Particle[n].weight*Gravity*DeltaTime;

            Particle[n].speed *= Particle[n].speed_chaos;

            Particle[n].v = Particle[n].ang * Particle[n].speed;
        }
        else
        {
            Particle[n].pos.x += DeltaTime * (Particle[n].v.x + Particle[n].chaos.x);
            Particle[n].pos.y += DeltaTime * (Particle[n].v.y + Particle[n].chaos.y);
            Particle[n].pos.z += DeltaTime * (Particle[n].v.z + Particle[n].chaos.z);
            Particle[n].angle += DeltaTime * Particle[n].spin;
            Particle[n].pos.y -= Particle[n].weight * Gravity * DeltaTime;
            Particle[n].speed *= Particle[n].speed_chaos;
            Particle[n].v = Particle[n].ang * Particle[n].speed;
        }

        /*if(fWindEffect != 0.0f && fWindPower != 0.0f)
        {
          Particle[n].pos += (DeltaTime*fWindEffect*fWindPower)*vWindDirection;
        }*/
        // bComplete = false;    // still have particles to run
    }

    // core.Trace("Delta: %d",DeltaTime);

    DeltaTimeSLE += DeltaTime;
    if (DeltaTimeSLE >= (EmissionTime + CurrentEmissionTimeRand))
    {
        // burn new particle
        if (EmitParticle())
        {
            DeltaTimeSLE = 0; // DeltaTimeSLE - EmissionTime;
        }
        CurrentEmissionTimeRand = static_cast<float>(EmissionTimeRand) * rand() / RAND_MAX;
    }

    if (!bRepeat)
    {
        nEmitted++;
        if (static_cast<int>(nEmitted) > ParticlesNum)
        {
            bComplete = true;
            for (n = 0; n < ParticlesNum; n++)
            {
                if (Particle[n].done)
                    continue;
                bComplete = false;
                break;
            }
        }
    }
}

void SEAFOAM_PS::SetDelay(int32_t _delay)
{
    int32_t n;
    Delay = _delay;
    if (Delay > 0)
    {
        for (n = 0; n < ParticlesNum; n++)
        {
            Particle[n].color = Particle[n].color & 0xffffff;
        }
    }
}

void SEAFOAM_PS::SetParticlesTracks(uint32_t DeltaTime)
{
    uint32_t color;
    uint32_t alpha;
    float val;
    int32_t n;

    for (n = 0; n < ParticlesNum; n++)
    {
        if (!Particle[n].live)
            continue;
        // alpha ----------------------------------------------
        val = GetTrackValue(&Visibility[0], Particle[n].time);
        alpha = static_cast<uint32_t>(static_cast<float>(0xff) * val);
        color = Particle[n].color & (0xffffff);

        // if(bColorInverse) {black = alpha; color = (black<<16)|(black<<8)|black;}

        color = ((alpha << 24) & 0xff000000) | color;

        Particle[n].color = color;

        // size -----------------------------------------------
        Particle[n].size = GetTrackValue(ParticleSize, Particle[n].time);

        // speed ----------------------------------------------
        Particle[n].speed = Particle[n].speedVal * GetTrackValue(ParticleSpeed, Particle[n].time);

        // spin -----------------------------------------------
        Particle[n].spin = Particle[n].spinVal * GetTrackValue(ParticleSpin, Particle[n].time);

        // angle ----------------------------------------------
        if (bTrackAngle)
            Particle[n].angle = GetTrackValue(ParticleAngle, Particle[n].time);

        // wind effect ----------------------------------------
        if (fWindEffect != 0.0f && fWindPower != 0.0f)
        {
            val = GetTrackValue(WindEffect, Particle[n].time) * fWindEffect;
            Particle[n].pos += (DeltaTime * val * fWindPower) * vWindDirection;
        }
    }
}

float SEAFOAM_PS::GetTrackValue(TRACK_EVENT *Track, int32_t Time)
{
    int32_t n;
    float v1, v2;
    int32_t t1, t2;

    if (Time < 0)
        return 0;

    for (n = 0; n < TRACK_EVENT_MAX; n++)
    {
        // if time - return value
        if (Time == Track[n].time)
            return Track[n].value;

        if (Track[n].time < 0)
        {
            // if no more keys - return previous value
            if (n == 0)
                return 0;
            return Track[n - 1].value;
        }
        // skip already processed keys
        if (Time > Track[n].time)
            continue;

        // set from value and time
        if (n == 0)
        {
            v1 = 0;
            t1 = 0;
        }
        else
        {
            v1 = Track[n - 1].value;
            t1 = Track[n - 1].time;
        }

        // set to value and time
        v2 = Track[n].value;
        t2 = Track[n].time;

        if (t1 == t2)
            return Track[n].value; // input error, double key

        return v1 + (Time - t1) * (v2 - v1) / (t2 - t1);
    }
    return 0;
}

bool SEAFOAM_PS::BuildTrack(TRACK_EVENT *Track, const std::string_view& section, const std::string_view& key)
{
    bool is_full_config = true;

    auto config = Config::Load(Constants::ConfigNames::particles());
    std::ignore = config.SelectSection(std::string(section));

    auto value_time_vec = config.Get<std::vector<std::vector<double>>>(std::string(key), {});
    for (int track_idx = 0, confix_size = std::size(value_time_vec); track_idx < TRACK_EVENT_MAX; track_idx++) {
        Track[track_idx].value = 0;
        Track[track_idx].time = -1;

        if (track_idx < confix_size && std::size(value_time_vec[track_idx]) >= 2) {
            Track[track_idx].value = value_time_vec[track_idx][0];
            Track[track_idx].time = value_time_vec[track_idx][1];
        } else {
            is_full_config = false;
            Track[track_idx].value = 0;
            Track[track_idx].time = -1;
        }
    }
    return is_full_config;
}

void SEAFOAM_PS::SetEmitter(CVECTOR p, CVECTOR a)
{
    Emitter = p;
    EmitterDirection = !a;
    /*
    for(n=0;n<ParticlesNum;n++)
    {
      Particle[n].ang.x = EmitterDirection.x + DirectionDeviation*(0.5f - (float)rand()/RAND_MAX);
      Particle[n].ang.y = EmitterDirection.y + DirectionDeviation*(0.5f - (float)rand()/RAND_MAX);
      Particle[n].ang.z = EmitterDirection.z + DirectionDeviation*(0.5f - (float)rand()/RAND_MAX);
    }
    */
}

void SEAFOAM_PS::LinkToObject(entid_t id, CVECTOR _LinkPos)
{
    int32_t n;
    bLinkEmitter = true;
    LinkObject = id;
    LinkPos = _LinkPos;
    LinkDir = EmitterDirection;
    LinkDirPos = LinkPos + LinkDir;

    COLLISION_OBJECT *pLink;
    pLink = static_cast<COLLISION_OBJECT *>(core.GetEntityPointer(LinkObject));
    if (pLink)
        Emitter = pLink->mtx * LinkPos;

    for (n = 0; n < ParticlesNum; n++)
    {
        Particle[n].pos.x = Emitter.x + ESpace * (0.5f - static_cast<float>(rand()) / RAND_MAX);
        Particle[n].pos.y = Emitter.y + ESpace * (0.5f - static_cast<float>(rand()) / RAND_MAX);
        Particle[n].pos.z = Emitter.z + ESpace * (0.5f - static_cast<float>(rand()) / RAND_MAX);
    }
    UpdateVertexBuffer();
}

bool SEAFOAM_PS::Complete()
{
    return bComplete;
}

/*
void SEAFOAM_PS::AddTrackPoint(CVECTOR pos)
{
    bUseFlowTrack = true;
    nFlowTrackSize++;
    pFlowTrack = (CVECTOR *)RESIZE(pFlowTrack,nFlowTrackSize*sizeof(CVECTOR));
    pFlowTrack[nFlowTrackSize - 1] = pos;
}*/

void SEAFOAM_PS::SetFlowTrack(uint32_t index)
{
    CVECTOR dest;
    if (Particle[index].flow_track_index >= nFlowTrackSize)
        return;
    dest = pFlowTrack[Particle[index].flow_track_index];
    dest = dest - Particle[index].pos;
    Particle[index].ang = !dest;
    const auto dist = ~dest;
    if (dist < fTrackPointRadius)
    {
        Particle[index].flow_track_index++;
    }
    // if(index==0)core.Trace("track: %d",Particle[index].flow_track_index);
}

void SEAFOAM_PS::UseSurface(entid_t surface_id)
{
    bLayOnSurface = true;
    SurfaceID = surface_id;
}
