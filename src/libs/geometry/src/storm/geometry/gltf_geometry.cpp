#include "gltf_geometry.hpp"

#include "../../geometry_r.h"
#include "../../rdf.h"

#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_USE_CPP14
#include "tiny_gltf.h"

extern GEOM_SERVICE_R GSR;

namespace storm
{
class UnimplementedError : public std::runtime_error
{
public:
    UnimplementedError() : runtime_error("Function not implemented")
    {
    }
};

class GltfGeometry::Impl
{
public:
    Impl(tinygltf::Model model) : model_(std::move(model))
    {
    }

    void Initialize();

    tinygltf::Model model_;
    GEOS::MATERIAL material_;
    int32_t vertexBuffer_{};
    int32_t indexBuffer_{};
    int32_t vertexCount_;
    int32_t triangleCount_;
    bool initialized_ = false;
};

void GltfGeometry::Impl::Initialize()
{
    if (initialized_ || model_.meshes.empty())
    {
        return;
    }
    const auto &mesh = model_.meshes.front();
    const auto &buffer = model_.buffers[0].data;
    for (const auto &primitive : mesh.primitives)
    {
        const auto &indexAccessor = model_.accessors[primitive.indices];
        const auto &indexView = model_.bufferViews[indexAccessor.bufferView];
        indexBuffer_ = GSR.CreateIndexBuffer(indexView.byteLength);
        auto *triangles = static_cast<char*>(GSR.LockIndexBuffer(indexBuffer_));
        std::copy(buffer.begin() + indexView.byteOffset, buffer.begin() + indexView.byteOffset + indexView.byteLength, triangles);
        GSR.UnlockIndexBuffer(indexBuffer_);
        triangleCount_ = indexAccessor.count / 3;

        if (primitive.attributes.contains("POSITION"))
        {
            const auto &position = model_.accessors[primitive.attributes.at("POSITION")];
            const auto &positionBuffer = model_.bufferViews[position.bufferView];
            const auto &normal = model_.accessors[primitive.attributes.at("NORMAL")];
            const auto &normalBuffer = model_.bufferViews[normal.bufferView];
            const auto &texCoord = model_.accessors[primitive.attributes.at("TEXCOORD_0")];
            const auto &texCoordBuffer = model_.bufferViews[texCoord.bufferView];
            vertexCount_ = position.count;
            vertexBuffer_ = GSR.CreateVertexBuffer(0, sizeof(RDF_VERTEX0) * vertexCount_);
            auto *vertices = reinterpret_cast<RDF_VERTEX0*>(GSR.LockVertexBuffer(vertexBuffer_));
            const size_t stride = positionBuffer.byteStride > 0 ? positionBuffer.byteStride : sizeof(float) * 3;
            const size_t texCoordStride = texCoordBuffer.byteStride > 0 ? texCoordBuffer.byteStride : sizeof(float) * 2;
            for (size_t i = 0; i < vertexCount_; ++i)
            {
                vertices[i] = {};
                vertices[i].pos.x = *reinterpret_cast<const float*>(&buffer[positionBuffer.byteOffset + i * stride]);
                vertices[i].pos.y = *reinterpret_cast<const float*>(&buffer[positionBuffer.byteOffset + i * stride + 1 * sizeof(float)]);
                vertices[i].pos.z = *reinterpret_cast<const float*>(&buffer[positionBuffer.byteOffset + i * stride + 2 * sizeof(float)]);
                vertices[i].norm.x = *reinterpret_cast<const float*>(&buffer[normalBuffer.byteOffset + i * stride]);
                vertices[i].norm.y = *reinterpret_cast<const float*>(&buffer[normalBuffer.byteOffset + i * stride + 1 * sizeof(float)]);
                vertices[i].norm.z = *reinterpret_cast<const float*>(&buffer[normalBuffer.byteOffset + i * stride + 2 * sizeof(float)]);
                vertices[i].tu0 = *reinterpret_cast<const float*>(&buffer[texCoordBuffer.byteOffset + i * texCoordStride]);
                vertices[i].tv0 = *reinterpret_cast<const float*>(&buffer[texCoordBuffer.byteOffset + i * texCoordStride + sizeof(float)]);
                vertices[i].color = 0xFFFFFFFF;
                vertices[i].norm = CVECTOR{1.f, 0.f, 0.f};
            }
            GSR.UnlockVertexBuffer(vertexBuffer_);
        }


        const auto &material = model_.materials[primitive.material];

        material_ = {};
        material_.diffuse = 0.8f;

        if (material.pbrMetallicRoughness.baseColorTexture.index != -1)
        {
            material_.texture_type[0] = TEXTURE_BASE;
            const auto &texture = model_.textures[material.pbrMetallicRoughness.baseColorTexture.index];
            const auto &image = model_.images[texture.source];
            material_.texture[0] = GSR.CreateTexture(image.uri.c_str());
        }

        break;
    }

    initialized_ = true;
}

GltfGeometry::GltfGeometry(tinygltf::Model model)
    : impl_(std::make_unique<Impl>(std::move(model)))
{
}

GltfGeometry::~GltfGeometry()
{
}

int32_t GltfGeometry::FindName(const char *name) const
{
    // Gltf has no groups
    return -1;
}

void GltfGeometry::GetInfo(GEOS::INFO &i) const
{
    i = {};
    i.ntextures = impl_->model_.textures.size();
    i.nmaterials = impl_->model_.materials.size();
    i.nobjects = impl_->model_.meshes.size();
}

std::vector<GEOS::LABEL> GltfGeometry::GetGroupLabels(const std::string_view &group) const
{
    std::vector<LABEL> labels;
    for (const auto &scene: impl_->model_.scenes)
    {
        if (scene.name == group)
        {
            for (const auto &node : impl_->model_.nodes)
            {
                labels.emplace_back(LABEL{
                    .m = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1},
                    .flags = {},
                    .group_name = nullptr,
                    .name = node.name.c_str(),
                    .bones = {},
                    .weight = {},
                });
            }
        }
    }
    return labels;
}

int32_t GltfGeometry::FindLabelN(int32_t start_index, int32_t name_id)
{
    throw UnimplementedError();
}

int32_t GltfGeometry::FindLabelG(int32_t start_index, int32_t group_name_id)
{
    throw UnimplementedError();
}

void GltfGeometry::GetLabel(int32_t l, GEOS::LABEL &lb) const
{
    throw UnimplementedError();
}

void GltfGeometry::SetLabel(int32_t l, const GEOS::LABEL &lb)
{
    throw UnimplementedError();
}

int32_t GltfGeometry::FindMaterialN(int32_t start_index, int32_t name_id)
{
    throw UnimplementedError();
}

int32_t GltfGeometry::FindMaterialG(int32_t start_index, int32_t group_name_id)
{
    throw UnimplementedError();
}

void GltfGeometry::GetMaterial(int32_t m, GEOS::MATERIAL &mt) const
{
    throw UnimplementedError();
}

void GltfGeometry::SetMaterial(int32_t m, const GEOS::MATERIAL &mt)
{
    throw UnimplementedError();
}

int32_t GltfGeometry::FindObjN(int32_t start_index, int32_t name_id)
{
    throw UnimplementedError();
}

int32_t GltfGeometry::FindObjG(int32_t start_index, int32_t group_name_id)
{
    throw UnimplementedError();
}

void GltfGeometry::GetObj(int32_t o, GEOS::OBJECT &ob) const
{
    throw UnimplementedError();
}

void GltfGeometry::SetObj(int32_t o, const GEOS::OBJECT &ob)
{
    throw UnimplementedError();
}

int32_t GltfGeometry::FindLightN(int32_t start_index, int32_t name_id)
{
    throw UnimplementedError();
}

int32_t GltfGeometry::FindLightG(int32_t start_index, int32_t group_name_id)
{
    throw UnimplementedError();
}

void GltfGeometry::GetLight(int32_t l, GEOS::LIGHT &lt) const
{
    throw UnimplementedError();
}

void GltfGeometry::SetLight(int32_t l, const GEOS::LIGHT &lt)
{
    throw UnimplementedError();
}

void GltfGeometry::Draw(const GEOS::PLANE *pl, int32_t np, GEOS::MATERIAL_FUNC mtf) const
{
    impl_->Initialize();
    GSR.SetIndexBuffer(impl_->indexBuffer_);
    GSR.SetVertexBuffer(sizeof(RDF_VERTEX0), impl_->vertexBuffer_);
    GSR.SetMaterial(impl_->material_);
    GSR.DrawIndexedPrimitive(0, impl_->vertexCount_, sizeof(RDF_VERTEX0), 0, impl_->triangleCount_);
}

float GltfGeometry::Trace(GEOS::VERTEX &src, GEOS::VERTEX &dst)
{
    throw UnimplementedError();
}

bool GltfGeometry::Clip(const GEOS::PLANE *planes, int32_t nplanes, const GEOS::VERTEX &center, float radius,
                        GEOS::ADD_POLYGON_FUNC addpoly)
{
    throw UnimplementedError();
}

bool GltfGeometry::GetCollisionDetails(GEOS::TRACE_INFO &ti) const
{
    throw UnimplementedError();
}

int32_t GltfGeometry::FindTexture(int32_t start_index, int32_t name_id)
{
    throw UnimplementedError();
}

int32_t GltfGeometry::GetTexture(int32_t tx) const
{
    throw UnimplementedError();
}

const char *GltfGeometry::GetTextureName(int32_t tx) const
{
    throw UnimplementedError();
}

int32_t GltfGeometry::GetVertexBuffer(int32_t vb) const
{
    throw UnimplementedError();
}

int32_t GltfGeometry::GetIndexBuffer() const
{
    throw UnimplementedError();
}

} // namespace storm
