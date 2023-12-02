#pragma once

#include "geos.h"

namespace tinygltf
{
    class Model;
} // namespace tinygltf

namespace storm {

class GltfGeometry : public GEOS {
  public:
    GltfGeometry(tinygltf::Model model);
    ~GltfGeometry() override;

    int32_t FindName(const char *name) const override;
    void GetInfo(INFO &i) const override;
    std::vector<LABEL> GetGroupLabels(const std::string_view &group) const override;
    int32_t FindLabelN(int32_t start_index, int32_t name_id) override;
    int32_t FindLabelG(int32_t start_index, int32_t group_name_id) override;
    void GetLabel(int32_t l, LABEL &lb) const override;
    void SetLabel(int32_t l, const LABEL &lb) override;
    int32_t FindMaterialN(int32_t start_index, int32_t name_id) override;
    int32_t FindMaterialG(int32_t start_index, int32_t group_name_id) override;
    void GetMaterial(int32_t m, MATERIAL &mt) const override;
    void SetMaterial(int32_t m, const MATERIAL &mt) override;
    int32_t FindObjN(int32_t start_index, int32_t name_id) override;
    int32_t FindObjG(int32_t start_index, int32_t group_name_id) override;
    void GetObj(int32_t o, OBJECT &ob) const override;
    void SetObj(int32_t o, const OBJECT &ob) override;
    int32_t FindLightN(int32_t start_index, int32_t name_id) override;
    int32_t FindLightG(int32_t start_index, int32_t group_name_id) override;
    void GetLight(int32_t l, LIGHT &lt) const override;
    void SetLight(int32_t l, const LIGHT &lt) override;
    void Draw(const PLANE *pl, int32_t np, MATERIAL_FUNC mtf) const override;
    float Trace(VERTEX &src, VERTEX &dst) override;
    bool Clip(const PLANE *planes, int32_t nplanes, const VERTEX &center, float radius,
              ADD_POLYGON_FUNC addpoly) override;
    bool GetCollisionDetails(TRACE_INFO &ti) const override;
    int32_t FindTexture(int32_t start_index, int32_t name_id) override;
    int32_t GetTexture(int32_t tx) const override;
    const char *GetTextureName(int32_t tx) const override;
    int32_t GetVertexBuffer(int32_t vb) const override;
    int32_t GetIndexBuffer() const override;

  private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace storm
