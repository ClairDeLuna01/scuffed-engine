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
using namespace EngineGlobals;

#include "glm/glm.hpp"

using namespace glm;

typedef std::shared_ptr<class Scene> ScenePtr;

class Scene
{
    private:
    std::deque<ShaderProgramPtr> shaders;
    std::deque<MaterialPtr> materials;
    std::unordered_map<std::string, SkyboxPtr> skyboxes;

    constexpr static size_t MAX_LIGHTS = 10;
    Light lights[MAX_LIGHTS];
    size_t lightCount = 0;
    GameObjectPtr root;
    std::string name;

    u32 uboLights;
    u32 lightsIndex = 0;

    void addShader(ShaderProgramPtr shader);

    void addMaterial(MaterialPtr material);

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
};
