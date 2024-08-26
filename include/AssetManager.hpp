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

    static AssetManager &getInstance()
    {
        static AssetManager instance;
        return instance;
    }
};