#pragma once

#include "common_camera.h"

class ENTITY_STATE;
class ENTITY_STATE_GEN;
class VDX9RENDER;

class FREE_CAMERA final : public COMMON_CAMERA
{
  public:
    FREE_CAMERA();
    ~FREE_CAMERA() override;

    bool Init() override;
    void Move(uint32_t real_delta);
    void Execute(uint32_t real_delta) override;
    bool CreateState(ENTITY_STATE_GEN *state_gen) const;
    bool LoadState(ENTITY_STATE *state);

    void Save(CSaveLoad *pSL) override;
    void Load(CSaveLoad *pSL) override;

  private:
    CVECTOR position_{};
    CVECTOR angle_{};

    bool cameraOverride_ = false;
};
