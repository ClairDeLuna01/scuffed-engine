#pragma once
#include <bitset>
#include <memory>

#include "texture.hpp"
#include "typedef.hpp"

using RenderLayerPtr = std::shared_ptr<class RenderLayer>;
using PostProcessLayerPtr = std::shared_ptr<class PostProcessLayer>;

class RenderLayer : public std::enable_shared_from_this<RenderLayer>
{
  protected:
    u32 ID;
    bool depthWrite = false;
    bool depthTest = true;
    PostProcessLayerPtr postProcessLayer = nullptr;

  public:
    RenderLayer(u32 id, bool depthWrite, bool depthTest, PostProcessLayerPtr ppLayer = nullptr)
        : ID(id), depthWrite(depthWrite), depthTest(depthTest), postProcessLayer(ppLayer)
    {
    }

    u32 getID()
    {
        return ID;
    }

    bool getDepthWrite()
    {
        return depthWrite;
    }

    bool getDepthTest()
    {
        return depthTest;
    }

    virtual void render();

    void setPostProcessLayer(PostProcessLayerPtr ppLayer)
    {
        postProcessLayer = ppLayer;
    }

    static RenderLayerPtr DEFAULT;
};

class DefaultRenderLayer : public RenderLayer
{
  public:
    DefaultRenderLayer(PostProcessLayerPtr pplayer = nullptr) : RenderLayer(0, true, true, pplayer)
    {
    }

    void render() override;
};

class PostProcessLayer
{
  private:
    u8 fboInputMask;
    u32 fboOutputID;
    u32 fboBlitMask;
    bool clear = false;

  public:
    PostProcessLayer(u8 fboInputMask, u32 fboOutputID, u32 fboBlitMask, bool clear = false)
        : fboInputMask(fboInputMask), fboOutputID(fboOutputID), fboBlitMask(fboBlitMask), clear(clear)
    {
        if (fboOutputID >= FBO_N)
        {
            std::cerr << "Invalid FBO ID: " << fboOutputID << std::endl;
            exit(1);
        }
    }

    void bind()
    {
        fbos[fboOutputID]->bind();
        if (clear)
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        for (u8 i = 0; i < 8; i++)
        {
            glActiveTexture(GL_TEXTURE0 + i + 16);
            if (fboInputMask & (1 << i))
            {
                // std::cout << "binding fbo " << (int)i << " to texture unit " << (int)(i + 16) << std::endl;
                fbos[i]->bindTexture();
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }
        glActiveTexture(GL_TEXTURE0);
    }

    void blit()
    {
        for (u8 i = 0; i < 8; i++)
        {
            u32 masked = fboBlitMask & (0b111 << (i * 3));
            u32 value = masked >> (i * 3);
            if (value)
            {
                // std::cout << "supposed to blit to layer " << (int)i << " here" << std::endl;
                // std::cout << "value: " << std::bitset<3>(value) << std::endl;
                fbos[fboOutputID]->blit(fbos[i], ((value & 0b001) ? GL_COLOR_BUFFER_BIT : 0) |
                                                     ((value & 0b010) ? GL_DEPTH_BUFFER_BIT : 0) |
                                                     ((value & 0b100) ? GL_STENCIL_BUFFER_BIT : 0));
            }
        }

        // blit to screen
        u32 masked = fboBlitMask & (0b111 << 24);
        u32 value = masked >> 24;
        if (value)
        {
            // std::cout << "bliting fbo " << fboOutputID << " to screen" << std::endl;
            // std::cout << "value: " << std::bitset<3>(value) << std::endl;
            fbos[fboOutputID]->blit(((value & 0b001) ? GL_COLOR_BUFFER_BIT : 0) |
                                    ((value & 0b010) ? GL_DEPTH_BUFFER_BIT : 0) |
                                    ((value & 0b100) ? GL_STENCIL_BUFFER_BIT : 0));
        }
    }

    void unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};