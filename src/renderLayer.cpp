#include "RenderLayer.hpp"
#include "MeshManager.hpp"
#include "globals.hpp"
#include "mesh.hpp"
#include "scene.hpp"

RenderLayerPtr RenderLayer::DEFAULT = std::make_shared<DefaultRenderLayer>();

void RenderLayer::render()
{
    auto meshManager = getMeshManager();
    if (postProcessLayer == nullptr)
    {
        if (!depthWrite)
            glDepthMask(GL_FALSE);

        if (!depthTest)
        {
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }

        meshManager->Update(shared_from_this());
        if (!depthWrite)
            glDepthMask(GL_TRUE);

        if (!depthTest)
        {
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
        }
    }
    else
    {
        postProcessLayer->bind();
        if (!depthWrite)
            glDepthMask(GL_FALSE);

        if (!depthTest)
        {
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }

        meshManager->Update(shared_from_this());
        if (!depthWrite)
            glDepthMask(GL_TRUE);

        if (!depthTest)
        {
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
        }

        postProcessLayer->blit();
        postProcessLayer->unbind();
    }
}

void DefaultRenderLayer::render()
{
    auto meshManager = getMeshManager();
    if (postProcessLayer == nullptr)
    {
        if (!depthWrite)
            glDepthMask(GL_FALSE);
        if (!depthTest)
        {
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }
        meshManager->Update(shared_from_this());
        if (EngineGlobals::scene->getSkybox())
        {
            EngineGlobals::scene->getSkybox()->draw();
        }
        if (!depthWrite)
            glDepthMask(GL_TRUE);
        if (!depthTest)
        {
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
        }
    }
    else
    {
        postProcessLayer->bind();
        if (!depthWrite)
            glDepthMask(GL_FALSE);
        if (!depthTest)
        {
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }
        meshManager->Update(shared_from_this());
        if (EngineGlobals::scene->getSkybox())
        {
            EngineGlobals::scene->getSkybox()->draw();
        }
        if (!depthWrite)
            glDepthMask(GL_TRUE);
        if (!depthTest)
        {
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
        }
        postProcessLayer->blit();
        postProcessLayer->unbind();
    }
}