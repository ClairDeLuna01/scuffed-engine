#pragma once
#include "renderLayer.hpp"
#include "typedef.hpp"
#include <memory>
#include <vector>

typedef std::shared_ptr<class Mesh> MeshPtr;

class MeshManager
{
  private:
    std::vector<MeshPtr> meshes;

  public:
    MeshManager() = default;

    void addMesh(MeshPtr mesh)
    {
        meshes.push_back(mesh);
    }

    void Update(RenderLayerPtr renderLayer = RenderLayer::DEFAULT);
};

using MeshManagerPtr = std::shared_ptr<MeshManager>;

MeshManagerPtr getMeshManager();