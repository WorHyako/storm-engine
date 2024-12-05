#pragma once

#include "cannon_trace.h"

struct FRECT;

class ISLAND_BASE : public CANNON_TRACE_BASE
{
  public:
    virtual entid_t GetModelEID() = 0;

    virtual bool Check2DBoxDepth(CVECTOR vPos, CVECTOR vSize, float fAngY, float fMinDepth) = 0;
    virtual bool GetDepth(float x, float z, float *fRes) = 0;
    virtual bool GetDepthFast(float x, float z, float *fRes) = 0;

    virtual bool GetMovePoint(CVECTOR &vSrc, CVECTOR &vDst, CVECTOR &vRes) = 0;

    virtual entid_t GetSeabedEID() = 0;
};
