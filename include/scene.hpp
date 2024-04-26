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

    void addShader(ShaderProgramPtr shader)
    {
        shaders.push_back(shader);
    }

    void addMaterial(MaterialPtr material)
    {
        materials.push_back(material);
    }

  public:
    Scene() : root(createGameObject("root"))
    {
        glGenBuffers(1, &uboLights);
        glBindBuffer(GL_UNIFORM_BUFFER, uboLights);
        constexpr size_t uboSize = 336LL; // cf shader
        glBufferData(GL_UNIFORM_BUFFER, uboSize, nullptr, GL_STATIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboLights);
    };
    ~Scene()
    {
        glDeleteBuffers(1, &uboLights);
    }

    static ScenePtr Load(std::string path)
    {
        using namespace rapidxml;

        std::ifstream file(path);
        if (!file.is_open())
        {
            std::cerr << "Error: Could not open file " << path << std::endl;
            return nullptr;
        }

        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        xml_document<> doc;
        doc.parse<0>(&content[0]);

        xml_node<> *rootNode = doc.first_node("scene");
        if (!rootNode)
        {
            std::cerr << "Error: Could not find root node in scene file" << std::endl;
            return nullptr;
        }

        std::string sceneName = rootNode->first_attribute("name")->value();
        ScenePtr scene = std::make_shared<Scene>();
        memset(scene->lights, 0, sizeof(lights));

        std::unordered_map<std::string, ShaderProgramPtr> shaders;
        std::unordered_map<std::string, TexturePtr> textures;
        std::unordered_map<std::string, std::string> modelPaths;
        std::unordered_map<std::string, MaterialPtr> materials;

        struct LodDef
        {
            std::string path;
            float distance;
        };

        struct LodModelDef
        {
            std::vector<LodDef> lods;
        };

        std::unordered_map<std::string, LodModelDef> lodModels;
        std::unordered_map<std::string, GameObjectPtr> gameObjects;

        xml_node<> *ressourcesNode = rootNode->first_node("ressources");
        if (!ressourcesNode)
        {
            std::cerr << "Error: Could not find ressources node in scene file" << std::endl;
            return nullptr;
        }

        for (xml_node<> *child = ressourcesNode->first_node(); child; child = child->next_sibling())
        {
            std::string name = child->name();
            if (name == "shader")
            {
                std::string shaderName = child->first_attribute("name")->value();
                std::string vertPath = child->first_attribute("vertex")->value();
                std::string fragPath = child->first_attribute("fragment")->value();
                ShaderProgramPtr shader = newShaderProgram(vertPath, fragPath);
                scene->addShader(shader);
                shaders[shaderName] = shader;
            }
            else if (name == "texture")
            {
                std::string textureName = child->first_attribute("name")->value();
                std::string texturePath = child->first_attribute("path")->value();
                TexturePtr texture = loadTexture(texturePath.c_str());
                textures[textureName] = texture;
            }
            else if (name == "model")
            {
                std::string modelName = child->first_attribute("name")->value();
                std::string modelPath = child->first_attribute("path")->value();
                modelPaths[modelName] = modelPath;
            }
            else if (name == "LODmodel")
            {
                std::string modelName = child->first_attribute("name")->value();
                LodModelDef lodModel;
                for (xml_node<> *lod = child->first_node(); lod; lod = lod->next_sibling())
                {
                    LodDef lodDef;
                    lodDef.path = lod->first_attribute("path")->value();
                    lodDef.distance = std::stof(lod->first_attribute("distance")->value());
                    lodModel.lods.push_back(lodDef);
                }
                lodModels[modelName] = lodModel;
            }
            else if (name == "material")
            {
                std::string materialName = child->first_attribute("name")->value();
                std::string shaderName;
                std::vector<std::string> textureNames;
                for (xml_node<> *prop = child->first_node(); prop; prop = prop->next_sibling())
                {
                    std::string propName = prop->name();
                    if (propName == "shaderRef")
                    {
                        shaderName = prop->first_attribute("shader")->value();
                    }
                    else if (propName == "textureRef")
                    {
                        textureNames.push_back(prop->first_attribute("texture")->value());
                    }
                }
                MaterialPtr material = std::make_shared<Material>(shaders[shaderName]);
                for (auto &textureName : textureNames)
                {
                    material->addTexture(textures[textureName]);
                }

                materials[materialName] = material;
            }
            else if (name == "objectDef")
            {
                std::string objectName = child->first_attribute("name")->value();
                GameObjectPtr object = createGameObject(objectName);
                for (xml_node<> *prop = child->first_node(); prop; prop = prop->next_sibling())
                {
                    std::string propName = prop->name();
                    if (propName == "modelRef")
                    {
                        std::string modelName = prop->first_attribute("model")->value();
                        bool lod = false;
                        if (lodModels.find(modelName) != lodModels.end())
                            lod = true;

                        std::string materialName = prop->first_attribute("material")->value();

                        if (!lod)
                        {
                            object->addComponent<Mesh>(materials[materialName], modelPaths[modelName]);
                        }
                        else
                        {
                            std::vector<MeshPtr> lods;
                            std::vector<float> distances;
                            for (auto &lod : lodModels[modelName].lods)
                            {
                                MaterialPtr material = materials[materialName];
                                lods.push_back(loadMesh(material, lod.path.c_str()));
                                distances.push_back(lod.distance);
                            }
                            object->addComponent<LODMesh>(lods, distances);
                        }
                    }
                    else if (propName == "position")
                    {
                        vec3 position = parseVec3(prop->value());
                        object->getTransform().setPosition(position);
                    }
                    else if (propName == "rotation")
                    {
                        vec3 rotation = parseVec3(prop->value());
                        object->getTransform().setRotation(rotation);
                    }
                    else if (propName == "scale")
                    {
                        vec3 scale = parseVec3(prop->value());
                        object->getTransform().setScale(scale);
                    }
                    else if (propName == "script")
                    {
                        std::string scriptClassName = prop->first_attribute("className")->value();
                        ComponentPtr scriptComponent = object->addComponent(scriptClassName);

                        std::shared_ptr<Script> script = std::static_pointer_cast<Script>(scriptComponent);

                        for (xml_attribute<> *attr = prop->first_attribute(); attr; attr = attr->next_attribute())
                        {
                            if (std::string(attr->name()) == "className")
                                continue;

                            bool rslt = script->Serialize(attr->name(), attr->value());
                            if (!rslt)
                            {
                                std::cerr << "Error: Could not serialize property " << attr->name() << " with value "
                                          << attr->value() << " for script " << scriptClassName << std::endl;
                            }
                        }
                    }
                }

                gameObjects[objectName] = object;
            }
            else if (name == "cameraDef")
            {
                for (xml_node<> *prop = child->first_node(); prop; prop = prop->next_sibling())
                {
                    std::string propName = prop->name();
                    if (propName == "rotation")
                    {
                        camera->getTransform().setRotation(parseVec3(prop->value()));
                    }
                    else if (propName == "position")
                    {
                        camera->getTransform().setPosition(parseVec3(prop->value()));
                    }
                    else if (propName == "script")
                    {
                        std::string scriptClassName = prop->first_attribute("className")->value();
                        ComponentPtr scriptComponent = camera->addComponent(scriptClassName);

                        std::shared_ptr<Script> script = std::static_pointer_cast<Script>(scriptComponent);

                        for (xml_attribute<> *attr = prop->first_attribute(); attr; attr = attr->next_attribute())
                        {
                            if (std::string(attr->name()) == "className")
                                continue;

                            bool rslt = script->Serialize(attr->name(), attr->value());
                            if (!rslt)
                            {
                                std::cerr << "Error: Could not serialize property " << attr->name() << " with value "
                                          << attr->value() << " for script " << scriptClassName << std::endl;
                            }
                        }
                    }
                }
            }
            else if (name == "skybox")
            {
                std::string skyboxName = child->first_attribute("name")->value();
                std::string top = child->first_node("top")->first_attribute("path")->value();
                std::string bottom = child->first_node("bottom")->first_attribute("path")->value();
                std::string left = child->first_node("left")->first_attribute("path")->value();
                std::string right = child->first_node("right")->first_attribute("path")->value();
                std::string front = child->first_node("front")->first_attribute("path")->value();
                std::string back = child->first_node("back")->first_attribute("path")->value();
                CubeMapPtr cubeMap = loadCubeMap(std::array<std::string, 6>({right, left, top, bottom, front, back}));
                ShaderProgramPtr skyboxShader =
                    std::make_shared<ShaderProgram>("shader/skybox.vert", "shader/skybox.frag");
                MaterialPtr skyboxMaterial = std::make_shared<Material>(skyboxShader);
                scene->skyboxes[skyboxName] = std::make_shared<Skybox>(skyboxMaterial, cubeMap);
            }
        }

        xml_node<> *graphNode = rootNode->first_node("graph");
        if (!graphNode)
        {
            std::cerr << "Error: Could not find graph node in scene file" << std::endl;
            return nullptr;
        }

        std::stack<std::pair<GameObjectPtr, xml_node<> *>> stack;
        stack.push(std::make_pair(scene->root, graphNode));

        while (!stack.empty())
        {
            auto [parent, node] = stack.top();
            stack.pop();

            for (xml_node<> *child = node->first_node(); child; child = child->next_sibling())
            {
                std::string type = child->name();
                if (type == "object")
                {
                    std::string name = child->first_attribute("name")->value();
                    GameObjectPtr object = gameObjects[name];
                    if (!object)
                    {
                        std::cout << "Creating object " << name << std::endl;
                        object = createGameObject(name);
                        gameObjects[name] = object;
                    }

                    parent->addChild(object);

                    stack.push(std::make_pair(object, child));
                }
                else if (type == "camera")
                {
                    parent->addChild(camera);

                    for (xml_attribute<> *attr = child->first_attribute(); attr; attr = attr->next_attribute())
                    {
                        std::string name = attr->name();
                        if (name == "position")
                        {
                            camera->getTransform().setPosition(parseVec3(attr->value()));
                        }
                        else if (name == "rotation")
                        {
                            camera->getTransform().setRotation(parseVec3(attr->value()));
                        }
                    }

                    stack.push(std::make_pair(camera, child));
                }
                else if (type == "light")
                {
                    if (scene->lightCount >= MAX_LIGHTS)
                    {
                        std::cerr << "Error: Maximum number of lights reached" << std::endl;
                        continue;
                    }
                    Light light;
                    for (xml_attribute<> *attr = child->first_attribute(); attr; attr = attr->next_attribute())
                    {
                        std::string name = attr->name();
                        if (name == "position")
                        {
                            vec3 posLocal = parseVec3(attr->value());
                            light.position = parent->getObjectMatrix() * vec4(posLocal, 1);
                            // std::cout << "Light position: " << light.position << std::endl;
                        }
                        else if (name == "color")
                        {
                            light.color = parseColor(attr->value());
                        }
                        else if (name == "intensity")
                        {
                            light.intensity = std::stof(attr->value());
                        }
                    }
                    scene->lights[scene->lightCount++] = light;
                }
                else if (type == "skyboxRef")
                {
                    std::string skyboxName = child->first_attribute("skybox")->value();
                    skybox = scene->skyboxes[skyboxName];
                }
            }
        }

        // lights
        glBindBuffer(GL_UNIFORM_BUFFER, scene->uboLights);
        struct AllignedLight
        {
            vec3 position __attribute__((aligned(16)));
            vec3 color __attribute__((aligned(16)));
            f32 intensity __attribute__((aligned(4)));

            AllignedLight(Light light)
            {
                position = light.position;
                color = light.color;
                intensity = light.intensity;
            }

            AllignedLight()
            {
            }
        } __attribute__((aligned(16)));

        struct LightBlock
        {
            AllignedLight __attribute__((aligned(16))) lights[MAX_LIGHTS];
            int lightCount __attribute__((aligned(4)));
        } lightBlock;

        lightBlock.lightCount = scene->lightCount;
        for (size_t i = 0; i < scene->lightCount; i++)
        {
            lightBlock.lights[i] = AllignedLight(scene->lights[i]);
        }

        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(lightBlock), &lightBlock);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // scene->root->print();

        return scene;
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
        FixedUpdateWrapper();

        if (skybox)
        {
            skybox->draw();
        }
    }

    void FixedUpdate()
    {
        getPhysicsEngine()->Update();
        root->FixedUpdate();
    }

    void FixedUpdateWrapper()
    {
        static f32 accumulator = 0.0f;
        accumulator += deltaTime;
        while (accumulator >= fixedDeltaTime)
        {
            FixedUpdate();
            accumulator -= fixedDeltaTime;
        }
    }

    GameObjectPtr getRoot()
    {
        return root;
    }

    GameObjectPtr find(std::string name)
    {
        return root->find(name);
    }
};
