#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

using namespace glm;

FT_Library getFTLibrary();

struct Glyph
{
    unsigned int TextureID; // ID handle of the glyph texture
    ivec2 Size;             // Size of glyph
    ivec2 Bearing;          // Offset from baseline to left/top of glyph
    long Advance;           // Offset to advance to next glyph
};

class Font
{
  private:
    std::string fontName;
    std::unordered_map<FT_ULong, Glyph> characters;
    FT_Face face;
    int fontPixelHeight;

  public:
    Font(std::string filename, int fontPixelHeight = 48) : fontPixelHeight(fontPixelHeight)
    {
        FT_Library ft = getFTLibrary();
        if (FT_New_Face(ft, filename.c_str(), 0, &face))
        {
            throw std::runtime_error("Failed to load font");
        }

        FT_Set_Pixel_Sizes(face, 0, fontPixelHeight);

        fontName = face->family_name;

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

        FT_GlyphSlot slot = face->glyph; // <-- This is new

        for (FT_ULong c = 0; c < 128; c++)
        {
            // load character glyph
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }

            FT_Render_Glyph(slot, FT_RENDER_MODE_SDF);

            // if (c == 'A')
            // {
            //     for (u32 i = 0; i < face->glyph->bitmap.rows; i++)
            //     {
            //         for (u32 j = 0; j < face->glyph->bitmap.width; j++)
            //         {
            //             std::cout << (face->glyph->bitmap.buffer[i * face->glyph->bitmap.width + j] > 0 ? '#' : ' ');
            //         }
            //         std::cout << std::endl;
            //     }
            // }

            // generate texture
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED,
                         GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // now store character for later use
            Glyph character = {texture, glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                               glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top), face->glyph->advance.x};
            characters[c] = character;
        }
    }

    ~Font()
    {
        FT_Done_Face(face);

        for (auto &c : characters)
        {
            glDeleteTextures(1, &c.second.TextureID);
        }
    }

    Glyph getGlyph(FT_ULong c)
    {
        return characters[c];
    }

    std::string getFontName()
    {
        return fontName;
    }

    int getFontPixelHeight()
    {
        return fontPixelHeight;
    }
};

typedef std::shared_ptr<Font> FontPtr;