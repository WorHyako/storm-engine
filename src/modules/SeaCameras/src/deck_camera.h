// ==================================================================== ||
// Ship tie camera                                                      ||
// ==================================================================== ||
#pragma once

#include "common_camera.h"
#include "dx9render.h"
#include "model.h"
#include "vma.hpp"

#define D3DFVF_VERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)
#define AIM_SIZE 0.05f

class DECK_CAMERA : public COMMON_CAMERA
{
    struct VERTEX
    {
        D3DVECTOR v;
        D3DCOLOR color;
        float tu;
        float tv;
    };

    VERTEX tri[4];

    // parameters loaded from INI file //
    // --------------------------------------
    float fSensivityDistance, fSensivityHeightAngle, fSensivityAzimuthAngle;
    float fRockingX, fRockingZ;

    float MEN_STEP_UP;
    float MEN_STEP_MIN;
    float HEIGHT_MAX;
    float HEIGHT_MIN;
    float HEIGHT_STEP;
    float CAMERA_MAX_X, CAMERA_MIN_X;
    float DEFAULT_HEIGHT;
    //--------------------------------------

    CVECTOR g_gv0, g_gv1, g_gv2;

    MODEL *pModel;
    CVECTOR camera_pos, camera_ang;
    float h_eye;
    RECT Screen_Rect;
    int32_t lock_x;
    int32_t lock_y;
    int32_t vb_id;
    bool bLoad;
    NODE *pathNode;
    void SetStartPos();
    bool GetCrossXZ(CVECTOR &spos, CVECTOR &dv, CVECTOR &p1, CVECTOR &p2, CVECTOR &res);
    bool FindPath();
    void SetViewPoint(CVECTOR &cViewPoint);
    float MultiTrace(const CVECTOR &cvBase, const CVECTOR &cvDown, float fHBase);

  public:
    DECK_CAMERA();
    ~DECK_CAMERA() override;

    void SetCharacter(ATTRIBUTES *_pACharacter) override;

    bool Init() override;
    void Move(uint32_t DeltaTime);
    // void Realize(uint32_t Delta_Time);
    void Execute(uint32_t real_delta) override;
    bool CreateState(ENTITY_STATE_GEN *state_gen);
    bool LoadState(ENTITY_STATE *state);
    uint32_t AttributeChanged(ATTRIBUTES *pAttr) override;
    uint64_t ProcessMessage(MESSAGE &message) override;

    void Save(CSaveLoad *pSL) override;
    void Load(CSaveLoad *pSL) override;
};
