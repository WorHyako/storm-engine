#pragma once

#include "collide.h"
#include "dx9render.h"
#include "model.h"

#include <array>

#define ITEMS_INFO_QUANTITY 10

class BLADE : public Entity
{
#define FVF (D3DFVF_XYZ | D3DFVF_DIFFUSE)

    struct VERTEX
    {
        CVECTOR pos;
        uint32_t diffuse;
    };

#define WAY_LENGTH 64

    struct BLADE_INFO
    {
        int32_t color[2];               //    color of the blade
        float defLifeTime, lifeTime; //
        float time;                  // current time

        VERTEX vrt[WAY_LENGTH * 2]; // 16 stripped quads
        float vrtTime[WAY_LENGTH];

        BLADE_INFO();
        ~BLADE_INFO();
        void DrawBlade(VDX9RENDER *rs, unsigned int blendValue, MODEL *mdl, NODE *manNode);
        bool LoadBladeModel(MESSAGE &message);

        std::string id_;
        std::string locatorName_;
        std::string locatorNameActive_;

        entid_t parentEntityId_;

        bool active_ = false;
    };

    struct TIEITEM_INFO
    {
        int32_t nItemIndex;
        entid_t eid;
        char *locatorName;

        TIEITEM_INFO()
            : eid(0)
        {
            nItemIndex = -1;
            locatorName = nullptr;
        }

        ~TIEITEM_INFO()
        {
            Release();
        }

        void Release();
        void DrawItem(VDX9RENDER *rs, unsigned int blendValue, MODEL *mdl, NODE *manNode);
        bool LoadItemModel(const char *mdlName, const char *locName);
    };

    VDX9RENDER *rs;
    COLLIDE *col;
    entid_t man;
    unsigned int blendValue;

    std::array<BLADE_INFO, 2> blades_;

    entid_t gun;
    std::string gunLocator_;
    std::string gunLocatorActive_;
    bool isGunActive_ = false;

    TIEITEM_INFO items[ITEMS_INFO_QUANTITY];

    bool LoadBladeModel(MESSAGE &message);
    bool LoadGunModel(MESSAGE &message);
    void GunFire();

    void AddTieItem(MESSAGE &message);
    void DelTieItem(MESSAGE &message);
    void DelAllTieItem();
    int32_t FindTieItemByIndex(int32_t n);

  public:
    BLADE();
    ~BLADE() override;
    bool Init() override;

    void ProcessStage(Stage stage, uint32_t delta) override
    {
        switch (stage)
        {
        case Stage::realize:
            Realize(delta);
            break;
        }
    }

    void Realize(uint32_t Delta_Time);
    uint64_t ProcessMessage(MESSAGE &message) override;

    void SetEquipmentLocator(const std::string_view &equipment_id, const std::string_view &locator_name);
    void SetEquipmentActiveLocator(const std::string_view &equipment_id, const std::string_view &locator_name);

    void ShowEditor() override;
};
