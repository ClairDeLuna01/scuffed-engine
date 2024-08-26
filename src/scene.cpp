#include "scene.hpp"
#include "AssetManager.hpp"
#include "MeshManager.hpp"

#include "glm/glm.hpp"

using namespace glm;

void Scene::addShader(std::string shaderName, ShaderProgramPtr shader)
{
    shaders[shaderName] = shader;
}

void Scene::addMaterial(std::string materialName, MaterialPtr material)
{
    materials[materialName] = material;
}

Scene::Scene() : root(createGameObject("root"))
{
    glGenBuffers(1, &uboLights);
    glBindBuffer(GL_UNIFORM_BUFFER, uboLights);
    constexpr size_t uboSize = 368ULL; // cf shader
    glBufferData(GL_UNIFORM_BUFFER, uboSize, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboLights);
}

Scene::~Scene()
{
    glDeleteBuffers(1, &uboLights);
}

ScenePtr Scene::Load(std::string path)
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

    std::unordered_map<std::string, ShaderProgramPtr> &shaders = scene->shaders;
    std::unordered_map<std::string, TexturePtr> textures;
    std::unordered_map<std::string, std::string> modelPaths;
    std::unordered_map<std::string, MaterialPtr> &materials = scene->materials;

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

            auto attr = child->first_attribute("enableFBO");
            bool enableFBO = false;
            if (attr)
            {
                enableFBO = std::string(attr->value()) == "true";
            }

            ShaderProgramPtr shader = newShaderProgram(vertPath, fragPath, enableFBO);

            shaders[shaderName] = shader;
        }
        else if (name == "texture")
        {
            std::string textureName = child->first_attribute("name")->value();
            std::string texturePath = child->first_attribute("path")->value();
            TexturePtr texture = AssetManager::getInstance().loadTexture(texturePath.c_str());
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
        else if (name == "renderLayer")
        {
            bool depthWrite = false;
            bool depthTest = true;
            u32 layerID = std::stoi(child->first_attribute("layerID")->value());
            auto attr = child->first_attribute("depthWrite");
            if (attr)
            {
                depthWrite = std::string(attr->value()) == "true";
            }

            auto depthTestAttr = child->first_attribute("depthTest");
            if (depthTestAttr)
            {
                depthTest = std::string(depthTestAttr->value()) == "true";
            }

            PostProcessLayerPtr postProcessLayer = nullptr;

            xml_node<> *postProcessLayerNode = child->first_node();
            if (postProcessLayerNode)
            {
                u8 inputMask = std::stoi(postProcessLayerNode->first_attribute("bindMask")->value(), nullptr, 2);
                u32 outputID = std::stoi(postProcessLayerNode->first_attribute("mainFBO")->value());
                bool clear = false;

                auto clearAttr = postProcessLayerNode->first_attribute("clear");
                if (clearAttr)
                {
                    clear = std::string(clearAttr->value()) == "true";
                }

                u32 blitMask = 0;
                for (xml_node<> *blitMaskElement = postProcessLayerNode->first_node("blitMaskElement"); blitMaskElement;
                     blitMaskElement = blitMaskElement->next_sibling())
                {
                    std::string FBO_ID = blitMaskElement->first_attribute("FBO_ID")->value();
                    int color = 0;
                    int depth = 0;
                    int stencil = 0;

                    auto colorAttr = blitMaskElement->first_attribute("color");
                    if (colorAttr)
                        color = std::string(colorAttr->value()) == "true";

                    auto depthAttr = blitMaskElement->first_attribute("depth");
                    if (depthAttr)
                        depth = std::string(depthAttr->value()) == "true";

                    auto stencilAttr = blitMaskElement->first_attribute("stencil");
                    if (stencilAttr)
                        stencil = std::string(stencilAttr->value()) == "true";

                    u32 mask = color + (depth << 1) + (stencil << 2);
                    if (FBO_ID == "screen")
                    {
                        blitMask |= (mask << 24);
                    }
                    else
                    {
                        u32 fboID = std::stoi(FBO_ID);
                        blitMask |= (mask << (fboID * 3));
                    }
                }
                postProcessLayer = std::make_shared<PostProcessLayer>(inputMask, outputID, blitMask, clear);
            }

            if (layerID != 0)
            {
                scene->renderLayers.push_back(
                    std::make_shared<RenderLayer>(layerID, depthWrite, depthTest, postProcessLayer));
            }
            else
            {
                RenderLayer::DEFAULT->setPostProcessLayer(postProcessLayer);
            }
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
                    rapidxml::xml_attribute<char> *renderLayerAttr = prop->first_attribute("RenderLayerRef");
                    RenderLayerPtr renderLayer = RenderLayer::DEFAULT;
                    if (renderLayerAttr)
                    {
                        u32 n = std::stoi(renderLayerAttr->value());

                        auto it = std::find_if(scene->renderLayers.begin(), scene->renderLayers.end(),
                                               [n](RenderLayerPtr layer) { return layer->getID() == n; });
                        if (it != scene->renderLayers.end())
                        {
                            renderLayer = *it;
                        }
                        else
                        {
                            std::cerr << "Error: Could not find render layer with ID " << n << std::endl;
                        }
                    }

                    if (!lod)
                    {
                        object->addComponent<Mesh>(materials[materialName], modelPaths[modelName], renderLayer);
                    }
                    else
                    {
                        std::vector<MeshPtr> lods;
                        std::vector<float> distances;
                        for (auto &lod : lodModels[modelName].lods)
                        {
                            MaterialPtr material = materials[materialName];
                            lods.push_back(loadMesh(material, lod.path.c_str(), renderLayer));
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
                    scene->sceneCamera->getTransform().setRotation(parseVec3(prop->value()));
                }
                else if (propName == "position")
                {
                    scene->sceneCamera->getTransform().setPosition(parseVec3(prop->value()));
                }
                else if (propName == "script")
                {
                    std::string scriptClassName = prop->first_attribute("className")->value();
                    ComponentPtr scriptComponent = scene->sceneCamera->addComponent(scriptClassName);

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
            ShaderProgramPtr skyboxShader = std::make_shared<ShaderProgram>("shader/skybox.vert", "shader/skybox.frag");
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
                parent->addChild(scene->sceneCamera);

                for (xml_attribute<> *attr = child->first_attribute(); attr; attr = attr->next_attribute())
                {
                    std::string name = attr->name();
                    if (name == "position")
                    {
                        scene->sceneCamera->getTransform().setPosition(parseVec3(attr->value()));
                    }
                    else if (name == "rotation")
                    {
                        scene->sceneCamera->getTransform().setRotation(parseVec3(attr->value()));
                    }
                }

                stack.push(std::make_pair(scene->sceneCamera, child));
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
                        light.color = parseColorRGB(attr->value());
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
                scene->sceneSkybox = scene->skyboxes[skyboxName];
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

    struct AllignedDirectionalLight
    {
        vec3 direction __attribute__((aligned(16)));
        vec3 color __attribute__((aligned(16)));
        f32 intensity __attribute__((aligned(4)));

        AllignedDirectionalLight(DirectionalLight light)
        {
            direction = light.direction;
            color = light.color;
            intensity = light.intensity;
        }

        AllignedDirectionalLight(ShadowMappedDirectionalLight light)
        {
            Transform3D transform = light.getTransform();
            direction = transform.getForward();
            color = light.getColor();
            intensity = light.getIntensity();
        }

        AllignedDirectionalLight()
        {
        }
    } __attribute__((aligned(16)));

    struct LightBlock
    {
        AllignedLight __attribute__((aligned(16))) lights[MAX_LIGHTS];
        AllignedDirectionalLight __attribute__((aligned(16))) directionalLight;
        int lightCount __attribute__((aligned(4)));
    } lightBlock;

    lightBlock.lightCount = scene->lightCount;
    for (size_t i = 0; i < scene->lightCount; i++)
    {
        lightBlock.lights[i] = AllignedLight(scene->lights[i]);
    }

    // TODO: add directional light to scene file

    lightBlock.directionalLight = AllignedDirectionalLight(*getSun());

    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(lightBlock), &lightBlock);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // scene->root->print();

    // sort render layers
    std::sort(scene->renderLayers.begin(), scene->renderLayers.end(),
              [](RenderLayerPtr a, RenderLayerPtr b) { return a->getID() < b->getID(); });

    return scene;
}

void Scene::Start()
{
    EngineGlobals::camera = sceneCamera;

    root->Start();
    root->LateStart();
}

void Scene::Update()
{
    // ShaderProgramPtr shadowMapShader = shaders["shadowMap"];
    // if (shadowMapShader)
    // {
    //     shadowMapShader->use();
    //     auto sun = getSun();
    //     if (sun)
    //     {
    //         shadowMapShader->setUniform(UNIFORM_LOCATIONS::LIGHT_SPACE_MATRIX, sun->getLightSpaceMatrix());

    //         sun->bind();

    //         root->drawShadowMap();

    //         sun->unbind();

    //         static bool first = true;
    //         if (first)
    //         {
    //             first = false;
    //             sun->saveShadowMapToPPM("shadowMap.ppm");
    //         }
    //     }
    // }
    auto meshManager = getMeshManager();

    InputManager::stepCallback(EngineGlobals::window, EngineGlobals::deltaTime);

    root->EarlyUpdate();
    root->Update();

    // fbos[0]->bind();
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // meshManager->Update(RenderLayer::DEFAULT);
    // if (sceneSkybox)
    // {
    //     sceneSkybox->draw();
    // }
    // fbos[0]->blit(fbos[1]);
    // fbos[0]->blit(GL_DEPTH_BUFFER_BIT);
    // fbos[0]->unbind();

    // fbos[1]->bind();
    // glActiveTexture(GL_TEXTURE0 + 16);
    // fbos[0]->bindTexture();
    // meshManager->Update(renderLayers[1]);
    // fbos[1]->blit(GL_COLOR_BUFFER_BIT);
    // fbos[1]->unbind();

    // // static bool first = true;
    // // if (first)
    // // {
    // //     first = false;
    // //     fbo->drawToPPM("fbo.ppm");
    // //     fbo2->drawToPPM("fbo2.ppm");
    // // }

    // glDepthMask(GL_FALSE);
    // meshManager->Update(renderLayers[2]);
    // glDepthMask(GL_TRUE);

    // meshManager->Update(renderLayers[3]);

    for (auto &layer : renderLayers)
    {
        layer->render();
        static bool first = true;
        if (first)
        {
            fbos[0]->drawToPPM("fbo_0.ppm");
            fbos[1]->drawToPPM("fbo_1.ppm");
            first = false;
        }
    }

    root->LateUpdate();
    FixedUpdateWrapper();
}

void Scene::FixedUpdate()
{
    getPhysicsEngine()->Update();
    root->FixedUpdate();
}

void Scene::FixedUpdateWrapper()
{
    static f32 accumulator = 0.0f;
    accumulator += EngineGlobals::deltaTime;
    while (accumulator >= EngineGlobals::fixedDeltaTime)
    {
        FixedUpdate();
        accumulator -= EngineGlobals::fixedDeltaTime;
    }
}

GameObjectPtr Scene::getRoot()
{
    return root;
}

GameObjectPtr Scene::find(std::string name)
{
    return root->find(name);
}
