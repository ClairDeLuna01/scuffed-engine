#pragma once
#include <iostream>
#include <deque>
#include <string.h>

#include "typedef.hpp"
#include "utils.hpp"
#include "InputManager.hpp"
#include "GLutils.hpp"
#include "shader.hpp"
#include "gameobject.hpp"
#include "mesh.hpp"
#include "globals.hpp"
#include "DFAUtils.hpp"
using namespace EngineGlobals;

GENERATE_ENUM(ALPHABET,
              DUMMY,
              scene,
              ressources,
              shader,
              texture,
              model,
              LODmodel,
              LOD,
              material,
              shaderRef,
              textureRef,
              objectDef,
              modelRef,
              materialRef,
              pos,
              rot,
              sca,
              graph,
              object);

GENERATE_ENUM(STATES,

              INIT,
              SCENE,
              RESOURCES,
              SHADER,
              TEXTURE,
              MODEL,
              LODMODEL,
              LODSTATE,
              MATERIAL,
              SHADERREF,
              TEXTUREREF,
              OBJECTDEF,
              MODELREF,
              MATERIALREF,
              POSITION,
              ROTATION,
              SCALE,
              GRAPH,
              OBJECT);

class SceneXMLDFA
{
private:
    struct shaderPrefab
    {
        std::string name;
        std::string vertexShader;
        std::string fragmentShader;
    };

    struct texturePrefab
    {
        std::string name;
        std::string path;
    };

    struct modelPrefab
    {
        std::string name;
        std::string path;
    };

    struct LODPrefab
    {
        std::string path;
        f32 distance;
    };

    struct LODModelPrefab
    {
        std::string name;
        std::vector<LODPrefab> LODs;
    };

    struct materialPrefab
    {
        std::string name;
        std::shared_ptr<shaderPrefab> shader;
        std::vector<std::shared_ptr<texturePrefab>> textures;
    };

    struct gameObjectPrefab
    {
        std::string name;
        modelPrefab model = {"", ""};
        LODModelPrefab LODModel = {"", {}};
        std::shared_ptr<materialPrefab> material = nullptr;
        vec3 position = {0, 0, 0};
        vec3 rotation = {0, 0, 0};
        vec3 scale = {1, 1, 1};
        std::vector<gameObjectPrefab> children;
    };

    std::vector<std::shared_ptr<shaderPrefab>> shaders;
    std::vector<std::shared_ptr<texturePrefab>> textures;
    std::vector<modelPrefab> models;
    std::vector<LODModelPrefab> LODModels;
    std::vector<std::shared_ptr<materialPrefab>> materials;
    std::vector<gameObjectPrefab> gameObjects;

    std::shared_ptr<shaderPrefab> currentShader;
    std::shared_ptr<texturePrefab> currentTexture;
    modelPrefab currentModel;
    LODModelPrefab currentLODModel;
    std::shared_ptr<materialPrefab> currentMaterial;
    gameObjectPrefab currentGameObject;

    std::string currentString;

    DFATransition<INIT, scene, SCENE> t;

public:
    static const DFA dfa;
    static DFA load(std::string xmlSceneFile)
    {
        // FILE *file = fopen(xmlSceneFile.c_str(), "r");
        // if (!file)
        // {
        //     std::cerr << "Error opening file " << xmlSceneFile << std::endl;
        //     exit(1);
        // }

        // std::function<std::string(i32)> state_to_string = [](i32 state)
        // { return STATESN2S[(STATES)state]; };

        // std::function<std::string(i32)> input_to_string = [](i32 input)
        // {
        //     if (input == e)
        //     {
        //         return std::string("e");
        //     }
        //     return ALPHABETN2S[(ALPHABET)input];
        // };

        // union
        // {
        //     char MagicNumberStr[8];
        //     u64 MagicNumberInt;
        // } magicNumber;
        // fread(magicNumber.MagicNumberStr, 1, 5, file);
        // union
        // {
        //     char MagicNumberStr[8] = "<?xml";
        //     u64 MagicNumberInt;
        // } expectedMagicNumber;

        // if (magicNumber.MagicNumberInt != expectedMagicNumber.MagicNumberInt)
        // {
        //     std::cerr << "Invalid file format" << std::endl;
        //     exit(1);
        // }

        // char str[1000];

        // while (true)
        // {
        //     fscanf(file, "%[^<]", str);
        //     if (feof(file))
        //     {
        //         break;
        //     }

        //     fscanf(file, "<%[^>]>", str);
        //     std::string tagandattributes = str;
        //     std::string tag = tagandattributes.substr(0, tagandattributes.find(" "));
        //     std::string attributes = tagandattributes.substr(tagandattributes.find(" ") + 1);

        //     if (tag == "scene")
        //     {
        //     }
        // }
    }
};

const DFA SceneXMLDFA::dfa = DFA(
    0,
    std::vector<i32>{SCENE},

    DFATransition<INIT, scene, SCENE>(),
    DFATransition<SCENE, e, INIT>(),

    DFATransition<SCENE, ressources, RESOURCES>(),
    DFATransition<RESOURCES, e, SCENE>(),

    DFATransition<RESOURCES, shader, SHADER>(),
    DFATransition<SHADER, e, RESOURCES>(),

    DFATransition<RESOURCES, texture, TEXTURE>(),
    DFATransition<TEXTURE, e, RESOURCES>(),

    DFATransition<RESOURCES, model, MODEL>(),
    DFATransition<MODEL, e, RESOURCES>(),

    DFATransition<RESOURCES, LODmodel, LODMODEL>(),
    DFATransition<LODMODEL, LOD, LODSTATE>(),
    DFATransition<LODSTATE, e, LODMODEL>(),
    DFATransition<LODMODEL, e, RESOURCES>(),

    DFATransition<RESOURCES, material, MATERIAL>(),
    DFATransition<MATERIAL, shaderRef, SHADERREF>(),
    DFATransition<SHADERREF, e, MATERIAL>(),
    DFATransition<MATERIAL, textureRef, TEXTUREREF>(),
    DFATransition<TEXTUREREF, e, MATERIAL>(),
    DFATransition<MATERIAL, e, RESOURCES>(),
    DFATransition<SCENE, objectDef, OBJECTDEF>(),
    DFATransition<OBJECTDEF, modelRef, MODELREF>(),
    DFATransition<MODELREF, e, OBJECTDEF>(),
    DFATransition<OBJECTDEF, materialRef, MATERIALREF>(),
    DFATransition<MATERIALREF, e, OBJECTDEF>(),

    DFATransition<OBJECTDEF, pos, POSITION>(),
    DFATransition<POSITION, e, OBJECTDEF>(),
    DFATransition<OBJECTDEF, rot, ROTATION>(),
    DFATransition<ROTATION, e, OBJECTDEF>(),
    DFATransition<OBJECTDEF, sca, SCALE>(),
    DFATransition<SCALE, e, OBJECTDEF>(),
    DFATransition<OBJECTDEF, e, SCENE>(),

    DFATransition<SCENE, graph, GRAPH>(),
    DFATransition<GRAPH, e, SCENE>(),
    DFATransition<GRAPH, object, OBJECT>(),
    DFATransition<OBJECT, object, OBJECT>(),
    DFATransition<OBJECT, e, GRAPH>());

class Scene
{
private:
    std::deque<ShaderProgramPtr> shaders;
    GameObjectPtr root;

public:
    Scene() : root(createGameObject()){};
    ~Scene() = default;

    void addShader(ShaderProgramPtr shader)
    {
        shaders.push_back(shader);
    }

    void Start()
    {
        root->Start();
    }

    void Update()
    {
        InputManager::stepCallback(window, deltaTime);

        root->EarlyUpdate();
        root->Update();
        root->LateUpdate();

        if (skybox)
        {
            skybox->draw();
        }
    }

    static Scene parseScene(std::string path)
    {
        Scene scene;
    }
};
