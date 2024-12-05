#pragma once

#include "core.h"
#include "vai_objbase.h"

#include <storm/renderer/camera.hpp>

class COMMON_CAMERA : public storm::Camera
{
  public:
    COMMON_CAMERA() = default;
    ~COMMON_CAMERA() override = default;

    bool FindShip()
    {
        Assert(pACharacter);
        // get entity id from loaded ships
        auto &&entities = core.GetEntityIds("ship");
        for (auto ship : entities)
        {
            auto *pObj = static_cast<VAI_OBJBASE *>(core.GetEntityPointer(ship));
            if (pObj->GetACharacter() == pACharacter)
            {
                SetEID(pObj->GetModelEID());
                SetAIObj(pObj);
                return true;
            }
        }
        return false;
    }

    uint64_t ProcessMessage(MESSAGE &msg) override;

    MODEL *GetModelPointer() const
    {
        return static_cast<MODEL *>(core.GetEntityPointer(eidObject));
    }

    void SetAIObj(VAI_OBJBASE *_pAIObj)
    {
        pAIObj = _pAIObj;
    }

    VAI_OBJBASE *GetAIObj() const
    {
        return pAIObj;
    }

    void SetEID(entid_t pEID)
    {
        eidObject = pEID;
    }

    entid_t GetEID() const
    {
        return eidObject;
    }

    virtual void SetCharacter(ATTRIBUTES *_pACharacter)
    {
        pACharacter = _pACharacter;
    }

    void SetPerspective(float _fPerspective)
    {
        fPerspective = _fPerspective;
    }

    float GetPerspective() const
    {
        return fPerspective;
    }

    virtual void Save(CSaveLoad *pSL) = 0;
    virtual void Load(CSaveLoad *pSL) = 0;

  protected:
    ATTRIBUTES *pACharacter = nullptr;

    float fPerspective{1.285f};

  private:
    entid_t eidObject{invalid_entity};
    VAI_OBJBASE *pAIObj = nullptr;
};
