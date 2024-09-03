#include "AssetManager.hpp"
#include "component.hpp"
#include "gameObject.hpp"
#include "globals.hpp"
#include "mesh.hpp"

class Boid : public Script
{
  private:
    constexpr static float velocityLimit = 0.1f;

    MeshPtr mesh;

  public:
    void Start() override
    {
        // mesh = AssetManager::getInstance().getMesh("res/models/boid.obj");

        // gameObject->addComponent(mesh);
    }
};