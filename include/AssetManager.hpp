#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <unordered_map>

#include "font.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "texture.hpp"

class AssetManager
{
  private:
    std::unordered_map<std::string, TexturePtr> textures;
    std::unordered_map<std::string, FontPtr> fonts;
    std::unordered_map<std::string, MaterialPtr> materials;
    std::unordered_map<std::string, MeshPtr> meshes;

    bool fileExists(const std::string &filename)
    {
        return std::filesystem::exists(filename);
    }

    const std::string searchForRes(const std::string &filename)
    {
        std::string resPath = "res/";
        if (fileExists(resPath + filename))
        {
            return resPath + filename;
        }
        return filename;
    }

  public:
    AssetManager()
    {
        loadFont("res/fonts/default.ttf");
    }

    ~AssetManager() = default;

    FontPtr loadFont(std::string filepath, int fontPixelHeight = 48)
    {
        std::string filename = stripPath(filepath);
        if (fonts.find(filename) == fonts.end())
        {
            fonts[filename] = std::make_shared<Font>(filepath, fontPixelHeight);
        }
        return fonts[filename];
    }

    FontPtr loadFontByName(std::string fontName, int fontPixelHeight = 48)
    {
        std::string filename = searchForRes(fontName);
        return loadFont(filename, fontPixelHeight);
    }

    FontPtr getFont(std::string filename)
    {
        // first search by filename in keys
        if (fonts.find(filename) != fonts.end())
        {
            return fonts[filename];
        }
        // else search by font name
        for (auto &f : fonts)
        {
            if (f.second->getFontName() == filename)
            {
                return f.second;
            }
        }

        return loadFontByName(filename);
    }

    FontPtr getDefaultFont()
    {
        return fonts["default.ttf"];
    }

    TexturePtr loadTexture(std::string filepath)
    {
        std::string filename = stripPath(filepath);
        if (textures.find(filename) == textures.end())
        {
            textures[filename] = std::make_shared<Texture>(filepath.c_str());
        }
        return textures[filename];
    }

    TexturePtr loadTextureByName(std::string textureName)
    {
        std::string filename = searchForRes(textureName);
        return loadTexture(filename);
    }

    TexturePtr getTexture(std::string filename)
    {
        // first search by filename in keys
        if (textures.find(filename) != textures.end())
        {
            return textures[filename];
        }
        // else search by texture name
        for (auto &t : textures)
        {
            if (t.second->getName() == filename)
            {
                return t.second;
            }
        }

        return loadTextureByName(filename);
    }

    /*
        MaterialPtr loadMaterial(std::string shaderName)
    {
        if (materials.find(shaderName) == materials.end())
        {
            materials[shaderName] = std::make_shared<Material>(shaderName);
        }
        return materials[shaderName];
    }

    MaterialPtr getMaterial(std::string shaderName)
    {
        if (materials.find(shaderName) != materials.end())
        {
            return materials[shaderName];
        }
        return loadMaterial(shaderName);
    }

    MeshPtr loadMesh(MaterialPtr material, std::string filepath, RenderLayerPtr renderLayer = RenderLayer::DEFAULT)
    {
        std::string filename = stripPath(filepath);
        if (meshes.find(filename) == meshes.end())
        {
            meshes[filename] = std::make_shared<Mesh>(material, filepath, renderLayer);
        }
        return meshes[filename];
    }

    MeshPtr loadMeshByName(MaterialPtr material, std::string meshName,
                           RenderLayerPtr renderLayer = RenderLayer::DEFAULT)
    {
        std::string filename = searchForRes(meshName);
        return loadMesh(material, filename, renderLayer);
    }

    MeshPtr getMesh(std::string filename)
    {
        // first search by filename in keys
        if (meshes.find(filename) != meshes.end())
        {
            return meshes[filename];
        }
        // else search by mesh name
        for (auto &m : meshes)
        {
            if (m.second->getName() == filename)
            {
                return m.second;
            }
        }

        // introduces inconsistency in the API :/
        return nullptr;
    }
    */

    std::string stripPath(std::string filepath)
    {
        size_t found = filepath.find_last_of("/\\");
        return filepath.substr(found + 1);
    }

    static AssetManager &getInstance()
    {
        static AssetManager instance;
        return instance;
    }
};