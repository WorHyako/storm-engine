#pragma once

#include "ai_flow_graph.h"
#include "collide.h"
#include "dx9render.h"
#include "geometry.h"
#include "island_base.h"
#include "map_zipper.hpp"
#include "model.h"
#include "vma.hpp"

class ISLAND : public ISLAND_BASE
{
  public:
    ISLAND() = default;
    ~ISLAND() override;
    bool Init() override;
    void Realize(uint32_t Delta_Time);
    uint64_t ProcessMessage(MESSAGE &message) override;

    void ProcessStage(Stage stage, uint32_t delta) override
    {
        switch (stage)
        {
            // case Stage::execute:
            //    Execute(delta); break;
        case Stage::realize:
            Realize(delta);
            break;
            /*case Stage::lost_render:
                LostRender(delta); break;
            case Stage::restore_render:
                RestoreRender(delta); break;*/
        }
    }

    void SetDevice();

    // inherit functions COLLISION_OBJECT
    float Trace(const CVECTOR &src, const CVECTOR &dst) override;

    bool Clip(const PLANE *planes, int32_t nplanes, const CVECTOR &center, float radius,
              ADD_POLYGON_FUNC addpoly) override
    {
        return false;
    };

    const char *GetCollideMaterialName() override
    {
        return nullptr;
    };

    bool GetCollideTriangle(TRIANGLE &triangle) override
    {
        return false;
    };

    // inherit functions CANNON_TRACE_BASE
    float Cannon_Trace(int32_t iBallOwner, const CVECTOR &src, const CVECTOR &dst) override;

    // inherit functions ISLAND_BASE
    bool GetMovePoint(CVECTOR &vSrc, CVECTOR &vDst, CVECTOR &vRes) override;

    entid_t GetModelEID() override
    {
        return model_id;
    };

    entid_t GetSeabedEID() override
    {
        return seabed_id;
    };

    bool Check2DBoxDepth(CVECTOR vPos, CVECTOR vSize, float fAngY, float fMinDepth) override;
    bool GetDepth(float x, float z, float *fRes) override;
    bool GetDepthFast(float x, float z, float *fRes) override;

    void ShowEditor() override;

  private:
    bool SaveTga8(char *fname, uint8_t *pBuffer, uint32_t dwSizeX, uint32_t dwSizeY);

    // depth map section
    bool CreateHeightMap(const std::string_view &pDir, const std::string_view &pName);
    bool ActivateCamomileTrace(CVECTOR &vSrc);
    inline float GetDepthNoCheck(uint32_t iX, uint32_t iZ);

    bool Mount(const std::string_view &fname, const std::string_view &fdir, entid_t *eID);
    void Uninit();

    void CalcBoxParameters(CVECTOR &vBoxCenter, CVECTOR &vBoxSize);

    void SetName(const std::string_view &pIslandName)
    {
        sIslandName = pIslandName;
    };

    char *GetName()
    {
        return (char *)sIslandName.c_str();
    };

    void AddLocationModel(entid_t eid, const std::string_view &pIDStr, const std::string_view &pStr);

    std::string sIslandName;
    std::vector<entid_t> aForts;
    AIFlowGraph AIPath;
    entid_t AIFortEID{};

    FRECT rIsland{};
    bool bForeignModels = false;
    bool bDrawReflections = false;
    float fStepDX{};
    float fStepDZ{};
    float fStep1divDX{};
    float fStep1divDZ{};
    CVECTOR vBoxSize{};
    CVECTOR vBoxCenter{};
    CVECTOR vRealBoxSize{};
    uint32_t iDMapSize{};
    uint32_t iDMapSizeShift{};
    entid_t model_id = invalid_entity;
    entid_t seabed_id = invalid_entity;

    bool dynamicLightsOn = false; // dynamic lighting

    std::string cModelsDir;
    std::string cModelsID;
    std::string cFoamDir;

    float fDepthHeight[256]{};

    MapZipper mzDepth;

    std::vector<uint8_t> pDepthMap;

    VDX9RENDER *pRS = nullptr;
    VGEOMETRY *pGS = nullptr;
    COLLIDE *pCollide = nullptr;

    CMatrix mIslandOld;
    float fImmersionDepth{};
    float fImmersionDistance{};
    float fCurrentImmersion{};

    bool enableDebugView_ = false;
};
