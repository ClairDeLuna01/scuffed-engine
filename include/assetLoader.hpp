#pragma once
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>

#include <glm/glm.hpp>
using namespace glm;

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "typedef.hpp"

void loadMesh(const char *path, std::vector<uivec3> &indices, std::vector<vec3> &vertices, std::vector<vec3> &normals, std::vector<vec2> &uvs)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    if (scene->mNumMeshes == 0)
    {
        std::cerr << "ERROR::ASSIMP::No meshes found in the file" << std::endl;
        return;
    }

    aiMesh *mesh = scene->mMeshes[0];

    for (size_t i = 0; i < mesh->mNumVertices; i++)
    {
        vertices.push_back(vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
        normals.push_back(vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z));
        if (mesh->mTextureCoords[0])
        {
            uvs.push_back(vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y));
        }
    }

    indices.reserve(mesh->mNumFaces);
    for (size_t i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        if (face.mNumIndices != 3)
        {
            std::cerr << "ERROR::ASSIMP::Face is not a triangle" << std::endl;
            return;
        }
        indices.push_back(uivec3(face.mIndices[0], face.mIndices[1], face.mIndices[2]));
    }
}
