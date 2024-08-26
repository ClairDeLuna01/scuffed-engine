#include "AssetManager.hpp"
#include "component.hpp"
#include "gameObject.hpp"
#include "globals.hpp"
#include "scene.hpp"
#include "textRenderer.hpp"

#include <string>

class testScript : public Script
{
  private:
    TextRendererPtr textRenderer;
    [[Serialize]]
    std::string text;

  public:
    void Start() override
    {
        // AssetManager::getInstance().loadFont("res/fonts/SourceSansPro-Regular.ttf");
        gameObject->addComponent<TextRenderer>(text, vec2(EngineGlobals::windowSize) / 2.0f - vec2(50, -150),
                                               EngineGlobals::scene->getShader("text"));

        // std::cout << "TextRenderer added" << std::endl;
    }
};