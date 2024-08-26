#include "MeshManager.hpp"
#include "mesh.hpp"

MeshManagerPtr getMeshManager()
{
    static auto meshManager = std::make_shared<MeshManager>();
    return meshManager;
}

void MeshManager::Update(RenderLayerPtr renderLayer)
{
    // not a huge fan of this past "me"...
    // wtf is this comment
    // also yeah this is bad
    for (auto &mesh : meshes)
    {
        if (mesh->getGameObject()->getEnabled() && mesh->getRenderLayer()->getID() == renderLayer->getID())
        {
            mesh->ManualUpdate();
        }
    }
}