#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include <AssetManager.hpp>
#include <component.hpp>
#include <utils.hpp>

float pointsToPixels(float points);
float pixelsToPoints(float pixels);

class TagParser
{
  private:
    std::string str;
    std::string tagName;
    std::string tagValue;
    std::unordered_map<std::string, std::string> arguments;
    bool endTag = false;

    void parse();

  public:
    TagParser(std::string str) : str(str)
    {
        parse();
    }

    const std::string &getTagName() const
    {
        return tagName;
    }

    const std::string &getValue() const
    {
        return tagValue;
    }

    const std::string &getArgument(const std::string &arg) const
    {
        return arguments.at(arg);
    }

    bool hasArgument(const std::string &arg) const
    {
        return arguments.find(arg) != arguments.end();
    }

    const std::unordered_map<std::string, std::string> &getArguments() const
    {
        return arguments;
    }

    bool isEndTag() const
    {
        return endTag;
    }
};

// rich text renderer
class TextRenderer : public ComponentBase<TextRenderer>
{
  private:
    std::string str;
    vec2 screenPos;
    ShaderProgramPtr shader;
    u32 VAO, VBO;

    struct TextRendererParameters
    {
        float fontSize; // in points
        FontPtr font;
        vec4 solidColor;
        enum class gradientType
        {
            RAINBOW,
            TRANS,
            NONE,
        } gradient;
        f32 gradientFreq;
        vec4 backgroundColor;
        bool bold;
        bool italic;
        bool underline;
        bool strikethrough;
        bool outline;
        bool wave;
        f32 waveFreq;
        f32 waveAmp;
        f32 waveSpeed;
    };

    static constexpr vec4 gradientRainbow[] = {
        "#e81416"_rgb, // red
        "#ffa500"_rgb, // orange
        "#faeb36"_rgb, // yellow
        "#79c314"_rgb, // green
        "#487de7"_rgb, // blue
        "#4b369d"_rgb, // indigo
        "#70369d"_rgb, // violet
    };

    static constexpr vec4 gradientTrans[] = {
        "#5BCEFA"_rgb, // light blue
        "#F5A9B8"_rgb, // light pink
        "#FFFFFF"_rgb, // white
        "#F5A9B8"_rgb, // light pink
    };

    static constexpr i32 tag2int(const char *tag)
    {
        // tough day for clang
        if (__builtin_strcmp(tag, "color") == 0)
            return 0;
        if (__builtin_strcmp(tag, "font_size") == 0)
            return 1;
        if (__builtin_strcmp(tag, "font") == 0)
            return 2;
        if (__builtin_strcmp(tag, "bgcolor") == 0)
            return 3;
        if (__builtin_strcmp(tag, "gradient") == 0)
            return 4;
        if (__builtin_strcmp(tag, "b") == 0)
            return 5;
        if (__builtin_strcmp(tag, "i") == 0)
            return 6;
        if (__builtin_strcmp(tag, "u") == 0)
            return 7;
        if (__builtin_strcmp(tag, "s") == 0)
            return 8;
        if (__builtin_strcmp(tag, "o") == 0)
            return 9;
        if (__builtin_strcmp(tag, "wave") == 0)
            return 10;
        if (__builtin_strcmp(tag, "img") == 0)
            return 11;
        return -1;
    }

    static const TextRendererParameters defaultParams;

    TextRendererParameters params;

  public:
    TextRenderer(std::string str, vec2 screenPos, ShaderProgramPtr shader)
        : str(str), screenPos(screenPos), shader(shader), params(defaultParams)
    {
        // Configure VAO/VBO for texture quads
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // Set shader uniforms
        shader->use();
        shader->setUniform(UNIFORM_LOCATIONS::PROJECTION_MATRIX, ortho(0.0f, 800.0f, 0.0f, 600.0f));
    }

    // based on https://learnopengl.com/In-Practice/Text-Rendering
    void LateUpdate() override;

    void setScreenPos(vec2 screenPos)
    {
        this->screenPos = screenPos;
    }
};

typedef std::shared_ptr<TextRenderer> TextRendererPtr;

TextRendererPtr createTextRenderer(std::string str, vec2 screenPos, ShaderProgramPtr shader);