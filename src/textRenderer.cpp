#include "textRenderer.hpp"

const TextRenderer::TextRendererParameters TextRenderer::defaultParams = {
    12.0f,                                      // fontSize
    nullptr,                                    // font
    vec4(1.0f, 1.0f, 1.0f, 1.0f),               // color
    TextRendererParameters::gradientType::NONE, // gradient
    0.0f,                                       // gradientFreq
    vec4(0.0f, 0.0f, 0.0f, 0.0f),               // backgroundColor
    false,                                      // bold
    false,                                      // italic
    false,                                      // underline
    false,                                      // strikethrough
    false,                                      // outline
    false,                                      // wave
    1.0f,                                       // waveFreq
    1.0f,                                       // waveAmp
    1.0f,                                       // waveSpeed
};

float pointsToPixels(float points)
{
    return points * 96.0f / 72.0f;
}

float pixelsToPoints(float pixels)
{
    return pixels * 72.0f / 96.0f;
}

TextRendererPtr createTextRenderer(std::string str, vec2 screenPos, ShaderProgramPtr shader)
{
    return std::make_shared<TextRenderer>(str, screenPos, shader);
}

void TextRenderer::LateUpdate()
{
    //// Render text
    // Activate corresponding render state
    shader->use();
    glActiveTexture(GL_TEXTURE0);
    shader->setUniform(UNIFORM_LOCATIONS::FONT_COLOR, params.solidColor);
    shader->setUniform(UNIFORM_LOCATIONS::FONT_BACKGROUND_COLOR, params.backgroundColor);
    shader->setUniform(UNIFORM_LOCATIONS::FONT_BOLD, params.bold);
    shader->setUniform(UNIFORM_LOCATIONS::FONT_ITALIC, params.italic);
    shader->setUniform(UNIFORM_LOCATIONS::FONT_UNDERLINE, params.underline);
    shader->setUniform(UNIFORM_LOCATIONS::FONT_STRIKETHROUGH, params.strikethrough);
    shader->setUniform(UNIFORM_LOCATIONS::FONT_OUTLINE, params.outline);
    shader->setUniform(UNIFORM_LOCATIONS::TIME, (float)glfwGetTime());
    glBindVertexArray(VAO);
    glDepthMask(GL_FALSE);

    // Get the font
    FontPtr font = params.font;
    if (!font)
    {
        font = AssetManager::getInstance().getDefaultFont();
        params.font = font;
    }

    float x = screenPos.x;
    float y = screenPos.y;

    size_t displayCharCounter = 0;
    size_t gradientStart = 0;

    // for each character in the string
    for (size_t i = 0; i < str.size(); i++)
    {
        char c = str[i];

        // tag parsing
        if (c == '\\' && (c == '[' || c == ']'))
        {
            c = '[';
            i += 1;
        }
        else if (c == '[')
        {
            size_t j = str.find(']', i);
            if (j == std::string::npos)
            {
                std::cerr << "Tag not closed" << std::endl;
                break;
            }

            TagParser tagParser{str.substr(i + 1, j - i - 1)};

            switch (tag2int((char *)tagParser.getTagName().c_str()))
            {
            case tag2int("b"): // bold
                params.bold = !tagParser.isEndTag();
                shader->setUniform(UNIFORM_LOCATIONS::FONT_BOLD, params.bold);
                break;
            case tag2int("i"): // italic
                params.italic = !tagParser.isEndTag();
                shader->setUniform(UNIFORM_LOCATIONS::FONT_ITALIC, params.italic);
                break;
            case tag2int("u"): // underline
                params.underline = !tagParser.isEndTag();
                shader->setUniform(UNIFORM_LOCATIONS::FONT_UNDERLINE, params.underline);
                break;
            case tag2int("s"): // strikethrough
                params.strikethrough = !tagParser.isEndTag();
                shader->setUniform(UNIFORM_LOCATIONS::FONT_STRIKETHROUGH, params.strikethrough);
                break;
            case tag2int("o"): // outline
                params.outline = !tagParser.isEndTag();
                shader->setUniform(UNIFORM_LOCATIONS::FONT_OUTLINE, params.outline);
                break;
            case tag2int("color"): {
                if (tagParser.isEndTag()) // reset to default color
                {
                    params.solidColor = defaultParams.solidColor;
                    shader->setUniform(UNIFORM_LOCATIONS::FONT_COLOR, params.solidColor);
                    break;
                }

                // hex or user defined color
                if (tagParser.getValue()[0] == '#' ||
                    std::find(tagParser.getValue().begin(), tagParser.getValue().end(), ' ') !=
                        tagParser.getValue().end())
                {
                    params.solidColor = parseColorRGBA(tagParser.getValue().data());
                }
                // predefined color
                else
                {
                    if (text2Color.find(tagParser.getValue()) != text2Color.end())
                    {
                        params.solidColor = text2Color[tagParser.getValue()];
                    }
                    else
                    {
                        std::cerr << "Unknown color: " << tagParser.getValue() << std::endl;
                    }
                }

                shader->setUniform(UNIFORM_LOCATIONS::FONT_COLOR, params.solidColor);

                break;
            }
            case tag2int("font_size"):
                params.fontSize = std::stof(tagParser.getValue());
                break;

            case tag2int("font"): {
                FontPtr newFont = AssetManager::getInstance().getFont(tagParser.getValue());
                if (newFont)
                {
                    params.font = newFont;
                }
                else
                {
                    std::cerr << "Unknown font: " << tagParser.getValue() << std::endl;
                }
                break;
            }

            case tag2int("bgcolor"):
                if (tagParser.isEndTag()) // reset to default color
                {
                    params.backgroundColor = defaultParams.backgroundColor;
                    break;
                }

                if (tagParser.getValue()[0] == '#' ||
                    std::find(tagParser.getValue().begin(), tagParser.getValue().end(), ' ') !=
                        tagParser.getValue().end())
                {
                    params.backgroundColor = parseColorRGBA(tagParser.getValue().data());
                }
                else
                {
                    if (text2Color.find(tagParser.getValue()) != text2Color.end())
                    {
                        params.backgroundColor = text2Color[tagParser.getValue()];
                    }
                    else
                    {
                        std::cerr << "Unknown color: " << tagParser.getValue() << std::endl;
                    }
                }
                break;

            case tag2int("gradient"):
                if (tagParser.isEndTag())
                {
                    params.gradient = TextRendererParameters::gradientType::NONE;
                    shader->setUniform(UNIFORM_LOCATIONS::FONT_COLOR, params.solidColor);
                }
                else
                {
                    const std::string &type = tagParser.getArgument("type");
                    if (type == "rainbow")
                    {
                        params.gradient = TextRendererParameters::gradientType::RAINBOW;
                    }
                    else if (type == "trans")
                    {
                        params.gradient = TextRendererParameters::gradientType::TRANS;
                    }
                    else
                    {
                        std::cerr << "Unknown gradient: " << type << std::endl;
                    }

                    const std::string &freq = tagParser.getArgument("freq");
                    if (!freq.empty())
                    {
                        params.gradientFreq = std::stof(freq);
                    }
                    else
                    {
                        params.gradientFreq = 0.0f;
                    }

                    gradientStart = displayCharCounter;
                }
                break;

            case tag2int("wave"):
                if (tagParser.isEndTag())
                {
                    params.wave = false;
                }
                else
                {
                    params.wave = true;

                    const std::string &freq = tagParser.getArgument("freq");
                    params.waveFreq = freq.empty() ? 1.0f : std::stof(freq);

                    const std::string &amp = tagParser.getArgument("amp");
                    params.waveAmp = amp.empty() ? 1.0f : std::stof(amp);

                    const std::string &speed = tagParser.getArgument("speed");
                    params.waveSpeed = speed.empty() ? 1.0f : std::stof(speed);
                }
                break;

            default:
                std::cerr << "Unknown tag: " << tagParser.getTagName() << std::endl;
            }

            i = j;
            continue;
        }

        // new line
        else if (c == '\n' || (c == '\\' && i + 1 < str.size() && str[i + 1] == 'n'))
        {
            x = screenPos.x;
            constexpr float lineSpacing = 1.2f;
            y -= params.font->getFontPixelHeight() * lineSpacing;
            if (c == '\\')
            {
                i++;
            }
            continue;
        }

        // Get the glyph
        Glyph glyph = font->getGlyph(c);

        if (params.gradient != TextRendererParameters::gradientType::NONE)
        {
            size_t n = (displayCharCounter - gradientStart);
            if (params.gradientFreq != 0.0f)
            {
                f32 t = (float)glfwGetTime();
                n += (size_t)(t * params.gradientFreq);
            }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
            switch (params.gradient)
            {
            case TextRendererParameters::gradientType::RAINBOW:
                n = n % (sizeof(gradientRainbow) / sizeof(gradientRainbow[0]));
                shader->setUniform(UNIFORM_LOCATIONS::FONT_COLOR, gradientRainbow[n]);
                break;
            case TextRendererParameters::gradientType::TRANS:
                n = n % (sizeof(gradientTrans) / sizeof(gradientTrans[0]));
                shader->setUniform(UNIFORM_LOCATIONS::FONT_COLOR, gradientTrans[n]);
                break;
            }
#pragma GCC diagnostic pop
        }

        // std::cout << "Rendering " << c << std::endl;

        // Render the glyph
        float xpos = x + glyph.Bearing.x;
        float ypos = y - (glyph.Size.y - glyph.Bearing.y);

        if (params.wave)
        {
            constexpr float constAmp = 20.0f;
            constexpr float constFreq = 0.01f;

            float amp = constAmp * params.waveAmp;
            float freq = constFreq * params.waveFreq;

            ypos += amp * sin(freq * x + (float)glfwGetTime() * params.waveSpeed);
        }

        float w = glyph.Size.x;
        float h = glyph.Size.y;

        // update VBO for each character
        float vertices[6][4] = {
            {xpos, ypos + h, 0.0f, 0.0f}, {xpos, ypos, 0.0f, 1.0f},     {xpos + w, ypos, 1.0f, 1.0f},
            {xpos, ypos + h, 0.0f, 0.0f}, {xpos + w, ypos, 1.0f, 1.0f}, {xpos + w, ypos + h, 1.0f, 0.0f},
        };

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, glyph.TextureID);
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (glyph.Advance >> 6); // Bitshift by 6 to get value in pixels (2^6 = 64)

        displayCharCounter++;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDepthMask(GL_TRUE);
}

std::vector<std::string> strSplit(std::string str, std::string delim)
{
    std::vector<std::string> tokens;
    size_t pos = 0;
    while ((pos = str.find(delim)) != std::string::npos)
    {
        tokens.push_back(str.substr(0, pos));
        str.erase(0, pos + delim.length());
    }
    tokens.push_back(str);
    return tokens;
}

void TagParser::parse()
{
    std::vector<std::string> tokens = strSplit(str, " ");
    // first treat the tag
    std::string delim = "=";
    size_t pos = tokens[0].find(delim);
    if (pos == std::string::npos)
    {
        tagName = tokens[0];
    }
    else
    {
        tagName = tokens[0].substr(0, pos);
        tagValue = tokens[0].substr(pos + delim.length());
    }

    if (tagName[0] == '/')
    {
        tagName = tagName.substr(1);
        endTag = true;
    }

    // then treat the arguments
    for (size_t i = 1; i < tokens.size(); i++)
    {
        std::string arg = tokens[i];
        pos = arg.find(delim);
        if (pos == std::string::npos)
        {
            arguments[arg] = "";
        }
        else
        {
            arguments[arg.substr(0, pos)] = arg.substr(pos + delim.length());
        }
    }
}
