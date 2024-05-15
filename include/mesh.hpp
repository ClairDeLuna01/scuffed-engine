#pragma once

#include <memory>
#include <type_traits>
#include <vector>

#include "camera.hpp"
#include "globals.hpp"
#include "material.hpp"
#include "texture.hpp"
#include "transform3D.hpp"
#include "typedef.hpp"

#include "component.hpp"
#include "gameObject.hpp"

// using namespace EngineGlobals;

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <reactphysics3d/reactphysics3d.h>

using namespace glm;

class ShadowMappedDirectionalLight
{
  private:
    Transform3D transform;
    u32 shadowMapFBOID;
    u32 shadowMapTextureID;
    u32 shadowMapWidth;
    u32 shadowMapHeight;
    mat4 lightSpaceMatrix;
    vec3 color;
    f32 intensity;

    void genShadowMapFBO()
    {
        glGenFramebuffers(1, &shadowMapFBOID);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBOID);

        glGenTextures(1, &shadowMapTextureID);
        glBindTexture(GL_TEXTURE_2D, shadowMapTextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT,
                     GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        GLfloat borderColor[] = {1.0, 1.0, 1.0, 1.0};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMapTextureID, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void calculateLightSpaceMatrix()
    {
        f32 near_plane = 0.01f, far_plane = 100.0f;
        f32 orthoSize = 50.0f;
        lightSpaceMatrix =
            ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, near_plane, far_plane) * transform.getModel();
    }

  public:
    ShadowMappedDirectionalLight(Transform3D _transform, u32 _shadowMapWidth, u32 _shadowMapHeight,
                                 vec3 _color = vec3(1.0f), f32 _intensity = 1.0f)
        : transform(_transform), shadowMapWidth(_shadowMapWidth), shadowMapHeight(_shadowMapHeight), color(_color),
          intensity(_intensity)
    {
        genShadowMapFBO();
        calculateLightSpaceMatrix();
    }

    void bind()
    {
        glViewport(0, 0, shadowMapWidth, shadowMapHeight);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBOID);
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    void unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    u32 getShadowMapTextureID()
    {
        return shadowMapTextureID;
    }

    mat4 getLightSpaceMatrix()
    {
        return lightSpaceMatrix;
    }

    vec3 getColor()
    {
        return color;
    }

    f32 getIntensity()
    {
        return intensity;
    }

    Transform3D getTransform()
    {
        return transform;
    }

    void saveShadowMapToPPM(std::string filename)
    {
        u32 size = shadowMapWidth * shadowMapHeight;
        f32 *dataFloat = new f32[size];
        u8 *data = new u8[size];
        glBindTexture(GL_TEXTURE_2D, shadowMapTextureID);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, dataFloat);
        f32 min = 1.0f;
        f32 max = 0.0f;

        for (u32 i = 0; i < size; i++)
        {
            if (dataFloat[i] < min)
                min = dataFloat[i];
            if (dataFloat[i] > max)
                max = dataFloat[i];
        }

        for (u32 i = 0; i < size; i++)
        {
            data[i] = (u8)(255.0f * (dataFloat[i] - min) / (max - min));
        }

        std::ofstream file(filename, std::ios::binary);

        std::string header =
            "P5\n" + std::to_string(shadowMapWidth) + " " + std::to_string(shadowMapHeight) + "\n255\n";
        file.write(header.c_str(), header.size());
        file.write((char *)data, size);

        delete[] dataFloat;
    }
};

std::shared_ptr<DirectionalLight> getSun();

class ElementBufferObject
{
  private:
    GLenum mode = GL_TRIANGLES;
    GLenum usage = GL_STATIC_DRAW;

    void *data;
    u64 dataLength;

    GLenum type = GL_UNSIGNED_INT;
    u32 elementSize = sizeof(unsigned int);

    u64 offset = 0;

    GLuint bufferID;

    inline void genBuffer()
    {
        glGenBuffers(1, &bufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, dataLength * elementSize, data, GL_STATIC_DRAW);
    }

  public:
    ElementBufferObject(void *_data, u64 _dataLength) : data(_data), dataLength(_dataLength)
    {
        genBuffer();
    }
    ElementBufferObject(void *_data, u64 _dataLength, GLenum _type, u32 _elementSize, GLenum _usage, u64 _offset)
        : usage(_usage), data(_data), dataLength(_dataLength), type(_type), elementSize(_elementSize), offset(_offset)
    {
        genBuffer();
    }

    inline void deleteBuffer()
    {
        glDeleteBuffers(1, &bufferID);
    }

    inline void bind()
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferID);
    }

    inline void unbind()
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    inline void draw() const
    {
        glDrawElements(mode, dataLength, type, (void *)offset);
    }

    void update(void *_data)
    {
        data = _data;
    }
};

class VertexBufferObject
{
  private:
    GLint elementCount;
    u64 elementSize;
    GLuint location;
    GLenum type;
    GLboolean normalized = GL_FALSE;
    GLenum usage = GL_STATIC_DRAW;
    GLsizei stride = 0;

    void *data;
    u64 dataLength = 0;
    u64 offset = 0;

    GLuint bufferID;

  public:
    VertexBufferObject(GLint _elementCount, u64 _elementSize, GLuint _location, GLenum _type, void *_data,
                       u64 _dataLength)
        : elementCount(_elementCount), elementSize(_elementSize), location(_location), type(_type), data(_data),
          dataLength(_dataLength)
    {
    }

    inline void deleteBuffer()
    {
        glDeleteBuffers(1, &bufferID);
    }

    inline void genBuffer()
    {
        // go look there
        // https://stackoverflow.com/questions/21652546/what-is-the-role-of-glbindvertexarrays-vs-glbindbuffer-and-what-is-their-relatio
        glGenBuffers(1, &bufferID);
        if (bufferID == 0)
        {
            std::cerr << "Failed to create Vertex Attrib Object.\n";
            exit(EXIT_FAILURE);
        }
        glBindBuffer(GL_ARRAY_BUFFER, bufferID);
        glBufferData(GL_ARRAY_BUFFER, dataLength * elementCount * elementSize, data, GL_STATIC_DRAW);
    }

    inline void bind()
    {
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, bufferID);
        glVertexAttribPointer(location, elementCount, type, normalized, stride, (void *)offset);
    }

    inline void unbind()
    {
        glDisableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void update(void *_data)
    {
        data = _data;

        glBindBuffer(GL_ARRAY_BUFFER, bufferID);
        glBufferData(GL_ARRAY_BUFFER, dataLength * elementCount, data, GL_STATIC_DRAW);
    }
};

struct Ray
{
    vec3 origin;
    vec3 direction;
};

typedef std::shared_ptr<class Mesh> MeshPtr;

class Mesh : public ComponentBase<Mesh>, public std::enable_shared_from_this<Mesh>
{
  protected:
    MaterialPtr material;

    std::vector<VertexBufferObject> vbos;
    EBOptr ebo = nullptr;

    GLuint vaoID;

    std::vector<uivec3> indices;
    std::vector<vec3> vertices;
    std::vector<vec3> normals;
    std::vector<vec2> uvs;

    bool wireframe = false;

    // horrible cursed hack to have transparent objects render after opaque objects but before some other objects
    // since we call update multiple times on meshes, we need to keep track of the "render state" of the mesh
    enum class RENDER_STATE
    {
        OPAQUE,
        TRANSPARENT,
        POST_TRANSPARENT,

        RENDER_STATE_COUNT
    } renderState = RENDER_STATE::OPAQUE;

  public:
    Mesh(MaterialPtr _mat) : material(_mat)
    {
        glGenVertexArrays(1, &vaoID);
    }
    Mesh(MaterialPtr _mat, std::string filename) : material(_mat)
    {
        glGenVertexArrays(1, &vaoID);

        FromFile(filename.c_str(), indices, vertices, normals, uvs);

        EBOptr ebo = std::make_unique<ElementBufferObject>((void *)indices.data(), indices.size() * 3);

        VertexBufferObject vbo1(3, sizeof(f32), 0, GL_FLOAT, (void *)vertices.data(), vertices.size());
        VertexBufferObject vbo2(3, sizeof(f32), 1, GL_FLOAT, (void *)normals.data(), normals.size());
        VertexBufferObject vbo3(2, sizeof(f32), 2, GL_FLOAT, (void *)uvs.data(), uvs.size());

        setEBO(ebo);

        addVBO(vbo1);
        addVBO(vbo2);
        addVBO(vbo3);
    }

    Mesh(MaterialPtr _mat, std::vector<uivec3> _indices, std::vector<vec3> _vertices, std::vector<vec3> _normals,
         std::vector<vec2> _uvs)
        : material(_mat), indices(_indices), vertices(_vertices), normals(_normals), uvs(_uvs)
    {
        glGenVertexArrays(1, &vaoID);

        EBOptr ebo = std::make_unique<ElementBufferObject>((void *)indices.data(), indices.size() * 3);

        VertexBufferObject vbo1(3, sizeof(f32), 0, GL_FLOAT, (void *)vertices.data(), vertices.size());
        VertexBufferObject vbo2(3, sizeof(f32), 1, GL_FLOAT, (void *)normals.data(), normals.size());
        VertexBufferObject vbo3(2, sizeof(f32), 2, GL_FLOAT, (void *)uvs.data(), uvs.size());

        setEBO(ebo);

        addVBO(vbo1);
        addVBO(vbo2);
        addVBO(vbo3);
    }

    Mesh(MaterialPtr _mat, std::vector<uivec3> _indices, std::vector<vec3> _vertices, std::vector<vec3> _normals)
        : material(_mat), indices(_indices), vertices(_vertices), normals(_normals)
    {
        glGenVertexArrays(1, &vaoID);

        EBOptr ebo = std::make_unique<ElementBufferObject>((void *)indices.data(), indices.size() * 3);

        VertexBufferObject vbo1(3, sizeof(f32), 0, GL_FLOAT, (void *)vertices.data(), vertices.size());
        VertexBufferObject vbo2(3, sizeof(f32), 1, GL_FLOAT, (void *)normals.data(), normals.size());

        setEBO(ebo);

        addVBO(vbo1);
        addVBO(vbo2);
    }

    ~Mesh();

    void addVBO(VertexBufferObject &vbo);

    void setEBO(EBOptr &_ebo);

    virtual void bind();

    virtual void draw();

    virtual void unbind();

    template <typename T, std::enable_if_t<std::is_same<T, mat4>::value || std::is_same<T, vec3>::value ||
                                               std::is_same<T, vec4>::value || std::is_same<T, f32>::value ||
                                               std::is_same<T, i32>::value || std::is_same<T, u32>::value,
                                           bool> = true>
    MeshPtr setUniform(u32 location, const T &value);

    template <typename T, std::enable_if_t<std::is_same<T, f64>::value, bool> = true>
    MeshPtr setUniform(u32 location, const T &value);

    template <typename T,
              std::enable_if_t<std::is_same<T, i64>::value || std::is_same<T, i16>::value || std::is_same<T, i8>::value,
                               bool> = true>
    MeshPtr setUniform(u32 location, const T &value);

    template <typename T,
              std::enable_if_t<std::is_same<T, u64>::value || std::is_same<T, u16>::value || std::is_same<T, u8>::value,
                               bool> = true>
    MeshPtr setUniform(u32 location, const T &value);

    MeshPtr addTexture(TexturePtr &texture);
    MeshPtr addTexture(std::string filename);

    void bind(mat4 objMat)
    {
        using namespace EngineGlobals;
        material->use();
        material->getShader()->setUniform(UNIFORM_LOCATIONS::MODEL_MATRIX, objMat);
        material->getShader()->setUniform(UNIFORM_LOCATIONS::VIEW_MATRIX, getViewMatrix());
        material->getShader()->setUniform(UNIFORM_LOCATIONS::PROJECTION_MATRIX, projectionMatrix);
        material->getShader()->setUniform(UNIFORM_LOCATIONS::SCREEN_RESOLUTION, vec2(EngineGlobals::windowSize));
        material->getShader()->setUniform(UNIFORM_LOCATIONS::VIEW_POS, camera->getTransform().getPosition());

        Mesh::bind();
    }

    void bindTransparent(mat4 objMat)
    {
        bind(objMat);

        glActiveTexture(GL_TEXTURE0 + material->getTextureCount());
        getFBO()->bindTexture();
        material->getShader()->setUniform(UNIFORM_LOCATIONS::FRAMEBUFFER, (i32)material->getTextureCount());
    }

    void draw(mat4 objMat)
    {
        if (wireframe)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        bind(objMat);
        ebo->draw();
        unbind();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    void drawTransparent(mat4 objMat)
    {
        bindTransparent(objMat);
        ebo->draw();
        unbind();
    }

    void Update() override
    {
        bool transparentMaterial = material->transparentShader();
        bool postTransparentMaterial = material->postTransparentShader();
        if (!(transparentMaterial || postTransparentMaterial) && renderState == RENDER_STATE::OPAQUE)
            draw(getGameObject()->getObjectMatrix());

        else if (transparentMaterial && renderState == RENDER_STATE::TRANSPARENT)
            drawTransparent(getGameObject()->getObjectMatrix());

        else if (postTransparentMaterial && renderState == RENDER_STATE::POST_TRANSPARENT)
        {
            draw(getGameObject()->getObjectMatrix());
        }

        renderState = (RENDER_STATE)((((i32)renderState) + 1) % (i32)RENDER_STATE::RENDER_STATE_COUNT);
    }

    void Update(mat4 objMat)
    {
        draw(objMat);
    }

    bool meshIntersect(::Ray r, vec3 &intersectionPoint, vec3 &normal) const;

    // temporary until we set up assimp to load whole scenes at once
    static void FromFile(const char *path, std::vector<uivec3> &indices, std::vector<vec3> &vertices,
                         std::vector<vec3> &normals, std::vector<vec2> &uvs);

    friend class GameObject;
    friend class Helper;
    friend class rp3dTriangleMeshHelper;

    friend rp3d::TriangleMesh *toRP3DMesh(const MeshPtr &mesh);
};

MeshPtr loadMesh(MaterialPtr mat, std::string filename);

class LODMesh : public ComponentBase<LODMesh>
{
  private:
    std::vector<MeshPtr> meshes;
    std::vector<f32> distances;
    u32 currentMesh = 0;

  public:
    LODMesh(std::vector<MeshPtr> _meshes, std::vector<f32> _distances) : meshes(_meshes), distances(_distances)
    {
    }

    void Update() override
    {
        using namespace EngineGlobals;
        vec3 cameraPosition = camera->getTransform().getPosition();
        f32 d = distance(getGameObject()->getTransform().getPosition(), cameraPosition);
        for (size_t i = 0; i < distances.size(); i++)
        {
            if (d < distances[i])
            {
                currentMesh = i;
                meshes[i]->Update(getGameObject()->getObjectMatrix());
                return;
            }
        }
        currentMesh = distances.size() - 1;
        meshes.back()->Update(getGameObject()->getObjectMatrix());
    }

    MeshPtr getCurrentMesh()
    {
        return meshes[currentMesh];
    }
};

class Skybox : public Mesh
{
  private:
    CubeMapPtr cubeMap;

  public:
    Skybox(MaterialPtr _mat, CubeMapPtr _cubeMap) : Mesh(_mat), cubeMap(_cubeMap)
    {
        glGenVertexArrays(1, &vaoID);

        std::vector<uivec3> indices;
        std::vector<vec3> vertices;
        std::vector<vec3> normals;
        std::vector<vec2> uvs;

        FromFile("res/cube.obj", indices, vertices, normals, uvs);

        EBOptr ebo = std::make_unique<ElementBufferObject>((void *)indices.data(), indices.size() * 3);

        VertexBufferObject vbo1(3, sizeof(f32), 0, GL_FLOAT, (void *)vertices.data(), vertices.size());
        VertexBufferObject vbo2(3, sizeof(f32), 1, GL_FLOAT, (void *)normals.data(), normals.size());
        VertexBufferObject vbo3(2, sizeof(f32), 2, GL_FLOAT, (void *)uvs.data(), uvs.size());

        setEBO(ebo);

        addVBO(vbo1);
        addVBO(vbo2);
        addVBO(vbo3);
    }

    void bind() override
    {
        using namespace EngineGlobals;
        material->use();
        mat4 view = getViewMatrix();
        view[3] = vec4(0, 0, 0, 1);
        material->getShader()->setUniform(UNIFORM_LOCATIONS::VIEW_MATRIX, view);
        material->getShader()->setUniform(UNIFORM_LOCATIONS::PROJECTION_MATRIX, projectionMatrix);
        material->getShader()->setUniform(UNIFORM_LOCATIONS::MODEL_MATRIX, mat4(1.0f));

        glActiveTexture(GL_TEXTURE0);
        cubeMap->bind();
        // setUniform(500, cubeMap->getTextureID());

        Mesh::bind();
    }

    void addTexture(TexturePtr &texture) = delete;
    void addTexture(std::string filename) = delete;

    void draw() override
    {
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        glDisable(GL_CULL_FACE);
        bind();
        ebo->draw();
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        unbind();
    }
};

typedef std::shared_ptr<Skybox> SkyboxPtr;

SkyboxPtr loadSkybox(MaterialPtr mat, CubeMapPtr cubeMap);