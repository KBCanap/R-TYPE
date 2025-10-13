/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** settings
*/

#pragma once
#include "../../ecs/include/render/IRenderWindow.hpp"
#include <memory>
#include <string>

enum class ColorblindMode {
    None = 0,
    Protanopia = 1,
    Deuteranopia = 2,
    Tritanopia = 3
};

class Settings {
  public:
    static Settings &getInstance() {
        static Settings instance;
        return instance;
    }

    void setColorblindMode(ColorblindMode mode) {
        _colorblindMode = mode;
        _colorblindShader.reset();
    }
    ColorblindMode getColorblindMode() const { return _colorblindMode; }

    std::string getColorblindModeName() const {
        switch (_colorblindMode) {
        case ColorblindMode::None:
            return "None";
        case ColorblindMode::Protanopia:
            return "Protanopia";
        case ColorblindMode::Deuteranopia:
            return "Deuteranopia";
        case ColorblindMode::Tritanopia:
            return "Tritanopia";
        default:
            return "None";
        }
    }

    render::Color
    applyColorblindFilter(const render::Color &originalColor) const {
        if (_colorblindMode == ColorblindMode::None) {
            return originalColor;
        }

        float r = originalColor.r / 255.0f;
        float g = originalColor.g / 255.0f;
        float b = originalColor.b / 255.0f;

        float newR = r, newG = g, newB = b;

        switch (_colorblindMode) {
        case ColorblindMode::Protanopia:
            newR = 0.567f * r + 0.433f * g;
            newG = 0.558f * r + 0.442f * g;
            newB = 0.242f * g + 0.758f * b;
            break;
        case ColorblindMode::Deuteranopia:
            newR = 0.625f * r + 0.375f * g;
            newG = 0.700f * r + 0.300f * g;
            newB = 0.300f * g + 0.700f * b;
            break;
        case ColorblindMode::Tritanopia:
            newR = 0.950f * r + 0.050f * g;
            newG = 0.433f * g + 0.567f * b;
            newB = 0.475f * g + 0.525f * b;
            break;
        default:
            break;
        }

        newR = std::max(0.0f, std::min(1.0f, newR));
        newG = std::max(0.0f, std::min(1.0f, newG));
        newB = std::max(0.0f, std::min(1.0f, newB));

        return render::Color(static_cast<uint8_t>(newR * 255),
                             static_cast<uint8_t>(newG * 255),
                             static_cast<uint8_t>(newB * 255),
                             originalColor.a
        );
    }

    render::IShader *getColorblindShader(render::IRenderWindow &window) {
        if (_colorblindMode == ColorblindMode::None) {
            return nullptr;
        }

        if (!_colorblindShader) {
            _colorblindShader = window.createShader();

            const std::string fragmentShaderSource = R"(
                uniform sampler2D texture;
                uniform int mode;

                void main() {
                    vec4 color = texture2D(texture, gl_TexCoord[0].xy);
                    vec3 rgb = color.rgb;
                    vec3 result = rgb;

                    if (mode == 1) {
                        result.r = 0.567 * rgb.r + 0.433 * rgb.g;
                        result.g = 0.558 * rgb.r + 0.442 * rgb.g;
                        result.b = 0.242 * rgb.g + 0.758 * rgb.b;
                    } else if (mode == 2) {
                        result.r = 0.625 * rgb.r + 0.375 * rgb.g;
                        result.g = 0.700 * rgb.r + 0.300 * rgb.g;
                        result.b = 0.300 * rgb.g + 0.700 * rgb.b;
                    } else if (mode == 3) {
                        result.r = 0.950 * rgb.r + 0.050 * rgb.g;
                        result.g = 0.433 * rgb.g + 0.567 * rgb.b;
                        result.b = 0.475 * rgb.g + 0.525 * rgb.b;
                    }

                    gl_FragColor = vec4(result, color.a);
                }
            )";

            if (_colorblindShader->loadFromMemory(
                    fragmentShaderSource, render::ShaderType::Fragment)) {
                _colorblindShader->setUniform("texture", 0);
            } else {
                _colorblindShader.reset();
            }
        }

        if (_colorblindShader) {
            _colorblindShader->setUniform("mode",
                                          static_cast<int>(_colorblindMode));
        }

        return _colorblindShader.get();
    }

    bool hasColorblindShader() const { return _colorblindShader != nullptr; }

    void setSoundEnabled(bool enabled) { _soundEnabled = enabled; }
    bool isSoundEnabled() const { return _soundEnabled; }

    void setResolution(unsigned int width, unsigned int height) {
        _resolutionWidth = width;
        _resolutionHeight = height;
    }
    unsigned int getResolutionWidth() const { return _resolutionWidth; }
    unsigned int getResolutionHeight() const { return _resolutionHeight; }

  private:
    Settings()
        : _colorblindMode(ColorblindMode::None), _soundEnabled(true),
          _resolutionWidth(800), _resolutionHeight(600) {}

    ColorblindMode _colorblindMode;
    bool _soundEnabled;
    unsigned int _resolutionWidth;
    unsigned int _resolutionHeight;
    std::unique_ptr<render::IShader> _colorblindShader;

    Settings(const Settings &) = delete;
    Settings &operator=(const Settings &) = delete;
};