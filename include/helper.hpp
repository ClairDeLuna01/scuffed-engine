#pragma once

#include "Physics.hpp"
#include "component.hpp"
#include "gameObject.hpp"
#include "globals.hpp"
#include "scene.hpp"
#include "utils.hpp"

// virtual helper class
class Helper : public ComponentBase<Helper>
{
  protected:
    MeshPtr constructedMesh;

    virtual void constructMesh() = 0;

  public:
    void Start() override
    {
        constructMesh();

        if (constructedMesh)
        {
            constructedMesh->wireframe = true;
            gameObject->addComponent(constructedMesh);
        }
    }
};

class rp3dTriangleMeshHelper : public Helper
{
  private:
    rp3d::TriangleMesh *mesh;

  public:
    rp3dTriangleMeshHelper(rp3d::TriangleMesh *mesh) : mesh(mesh)
    {
    }

    void constructMesh() override
    {
        MaterialPtr material = EngineGlobals::scene->getMaterial("debugMat");
        std::vector<vec3> vertices;
        std::vector<vec3> normals;
        std::vector<uivec3> indices;

        vertices.reserve(mesh->getNbVertices());
        normals.reserve(mesh->getNbVertices());
        indices.reserve(mesh->getNbTriangles());

        for (size_t i = 0; i < mesh->getNbVertices(); i++)
        {
            rp3d::Vector3 vertex = mesh->getVertex(i);
            vertices.push_back(vec3(vertex.x, vertex.y, vertex.z));

            rp3d::Vector3 normal = mesh->getVertexNormal(i);
            normals.push_back(vec3(normal.x, normal.y, normal.z));
        }

        for (size_t i = 0; i < mesh->getNbTriangles(); i++)
        {
            uint32 v1, v2, v3;
            mesh->getTriangleVerticesIndices(i, v1, v2, v3);
            indices.push_back(uivec3(v1, v2, v3));
        }

        MeshPtr newMesh = std::make_shared<Mesh>(material, indices, vertices, normals);
        constructedMesh = newMesh;
    }
};