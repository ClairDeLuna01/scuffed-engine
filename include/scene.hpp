#pragma once
#include <deque>
#include <iostream>
#include <stack>
#include <string.h>
#include <unordered_map>
#include <vector>

#include "GLutils.hpp"
#include "Physics.hpp"
#include "gameObject.hpp"
#include "globals.hpp"
#include "inputManager.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "rapidxml/rapidxml.hpp"
#include "shader.hpp"
#include "typedef.hpp"
#include "utils.hpp"

#include "glm/glm.hpp"

using namespace glm;

typedef std::shared_ptr<class Scene> ScenePtr;

class Scene
{
  private:
    std::unordered_map<std::string, ShaderProgramPtr> shaders;
    std::unordered_map<std::string, MaterialPtr> materials;
    std::unordered_map<std::string, SkyboxPtr> skyboxes;

    constexpr static size_t MAX_LIGHTS = 10;
    Light lights[MAX_LIGHTS];
    size_t lightCount = 0;
    GameObjectPtr root;
    std::string name;

    u32 uboLights;
    u32 lightsIndex = 0;

    FBOPtr fbo = nullptr;
    FBOPtr fbo2 = nullptr;

    CameraPtr sceneCamera = std::make_shared<Camera>();
    SkyboxPtr sceneSkybox = nullptr;

    void addShader(std::string shaderName, ShaderProgramPtr shader);

    void addMaterial(std::string materialName, MaterialPtr material);

  public:
    Scene();
    ~Scene();

    static ScenePtr Load(std::string path);

    void Start();

    void Update();

    void FixedUpdate();

    void FixedUpdateWrapper();

    GameObjectPtr getRoot();

    GameObjectPtr find(std::string name);

    MaterialPtr getMaterial(std::string name)
    {
        MaterialPtr material = materials[name];
        if (!material)
        {
            std::cerr << "Material " << name << " not found" << std::endl;
        }
        return material;
    }

    FBOPtr getFBO()
    {
        return fbo;
    }
};