#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include "../../ecs/include/render/IRenderWindow.hpp"

class Settings {
public:
    static Settings& getInstance() {
        static Settings instance;
        return instance;
    }

    // Contrast settings
    void setContrast(float contrast) { _contrast = contrast; }
    float getContrast() const { return _contrast; }

    // Utility function to apply contrast to a color
    sf::Color applyContrast(const sf::Color& originalColor) const {
        float factor = _contrast;

        // Convert to normalized values
        float r = originalColor.r / 255.0f;
        float g = originalColor.g / 255.0f;
        float b = originalColor.b / 255.0f;

        // Apply contrast adjustment
        // Formula: newValue = ((oldValue - 0.5) * contrast) + 0.5
        r = ((r - 0.5f) * factor) + 0.5f;
        g = ((g - 0.5f) * factor) + 0.5f;
        b = ((b - 0.5f) * factor) + 0.5f;

        // Clamp values to [0, 1]
        r = std::max(0.0f, std::min(1.0f, r));
        g = std::max(0.0f, std::min(1.0f, g));
        b = std::max(0.0f, std::min(1.0f, b));

        // Convert back to 0-255 range
        return sf::Color(
            static_cast<sf::Uint8>(r * 255),
            static_cast<sf::Uint8>(g * 255),
            static_cast<sf::Uint8>(b * 255),
            originalColor.a  // Keep alpha unchanged
        );
    }

    // Utility function to apply contrast to a render::Color
    render::Color applyContrast(const render::Color& originalColor) const {
        float factor = _contrast;

        // Convert to normalized values
        float r = originalColor.r / 255.0f;
        float g = originalColor.g / 255.0f;
        float b = originalColor.b / 255.0f;

        // Apply contrast adjustment
        // Formula: newValue = ((oldValue - 0.5) * contrast) + 0.5
        r = ((r - 0.5f) * factor) + 0.5f;
        g = ((g - 0.5f) * factor) + 0.5f;
        b = ((b - 0.5f) * factor) + 0.5f;

        // Clamp values to [0, 1]
        r = std::max(0.0f, std::min(1.0f, r));
        g = std::max(0.0f, std::min(1.0f, g));
        b = std::max(0.0f, std::min(1.0f, b));

        // Convert back to 0-255 range
        return render::Color(
            static_cast<uint8_t>(r * 255),
            static_cast<uint8_t>(g * 255),
            static_cast<uint8_t>(b * 255),
            originalColor.a  // Keep alpha unchanged
        );
    }

    // Get contrast shader for true contrast effect
    sf::Shader* getContrastShader() {
        if (!_contrastShader) {
            _contrastShader = std::make_unique<sf::Shader>();

            // Fragment shader for contrast adjustment
            const std::string fragmentShaderSource = R"(
                uniform sampler2D texture;
                uniform float contrast;

                void main() {
                    vec4 color = texture2D(texture, gl_TexCoord[0].xy);

                    // Apply contrast: (color - 0.5) * contrast + 0.5
                    color.rgb = ((color.rgb - 0.5) * contrast) + 0.5;

                    // Clamp values to [0, 1]
                    color.rgb = clamp(color.rgb, 0.0, 1.0);

                    gl_FragColor = color;
                }
            )";

            if (_contrastShader->loadFromMemory(fragmentShaderSource, sf::Shader::Fragment)) {
                _contrastShader->setUniform("texture", sf::Shader::CurrentTexture);
            } else {
                // Fallback: shader loading failed, disable contrast
                _contrastShader.reset();
            }
        }

        if (_contrastShader) {
            _contrastShader->setUniform("contrast", _contrast);
        }

        return _contrastShader.get();
    }

    // Check if contrast shader is available
    bool hasContrastShader() const {
        return _contrastShader != nullptr;
    }

    // Sound settings
    void setSoundEnabled(bool enabled) { _soundEnabled = enabled; }
    bool isSoundEnabled() const { return _soundEnabled; }

    // Resolution settings
    void setResolution(unsigned int width, unsigned int height) {
        _resolutionWidth = width;
        _resolutionHeight = height;
    }
    unsigned int getResolutionWidth() const { return _resolutionWidth; }
    unsigned int getResolutionHeight() const { return _resolutionHeight; }

private:
    Settings() : _contrast(1.0f), _soundEnabled(true), _resolutionWidth(800), _resolutionHeight(600) {}

    float _contrast;
    bool _soundEnabled;
    unsigned int _resolutionWidth;
    unsigned int _resolutionHeight;
    std::unique_ptr<sf::Shader> _contrastShader;

    // Delete copy constructor and assignment operator
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;
};