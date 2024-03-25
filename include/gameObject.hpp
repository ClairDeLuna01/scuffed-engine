#pragma once

#include <memory>
#include <type_traits>
#include <vector>

#include "shader.hpp"
#include "typedef.hpp"
#include "transform3D.hpp"
#include "texture.hpp"

#include "globals.hpp"
#include "mesh.hpp"

using namespace EngineGlobals;

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace glm;

class gameObject;
typedef std::shared_ptr<gameObject> GameObjectPtr;

class gameObject : public std::enable_shared_from_this<gameObject>
{
private:
    std::vector<GameObjectPtr> children;
    GameObjectPtr parent = nullptr;

    mat4 objMat = mat4(1.0f);
    Mesh3DPtr mesh = nullptr;
    bool enabled = true;

public:
    gameObject(Mesh3DPtr _mesh) : mesh(_mesh)
    {
    }

    void addChild(GameObjectPtr child)
    {
        children.push_back(child);
        child->parent = shared_from_this();

        updateChildren();
    }

    void setParent(GameObjectPtr _parent)
    {
        parent = _parent;
        parent->addChild(shared_from_this());

        parent->updateChildren();
    }

    GameObjectPtr getParent()
    {
        return parent;
    }

    mat4 getObjectMatrix()
    {
        return objMat;
    }

    void setTransform(Transform3D _transform)
    {
        mesh->transform = _transform;
        objMat = (!parent) ? mesh->transform.getModel() : parent->objMat * mesh->transform.getModel();
        updateChildren();
    }

    void setEnabled(bool _enabled)
    {
        enabled = _enabled;
    }

    bool getEnabled()
    {
        return enabled;
    }

    void updateChildren()
    {
        for (auto &child : children)
        {
            child->objMat = objMat * child->mesh->transform.getModel();
            child->updateChildren();
        }
    }

    void draw()
    {
        if (!enabled)
            return;
        mesh->draw(objMat);
        for (auto &child : children)
        {
            child->draw();
        }
    }

    Transform3D &getTransform()
    {
        return mesh->transform;
    }
};

GameObjectPtr createGameObject(Mesh3DPtr mesh);