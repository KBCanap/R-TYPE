#pragma once
#include <memory>
#include <string>
#include "../../ecs/include/render/IRenderWindow.hpp"

enum class ColorblindMode {
    None = 0,           // No filter
    Protanopia = 1,     // Red-blind (1% of males)
    Deuteranopia = 2,   // Green-blind (1% of males)
    Tritanopia = 3      // Blue-blind (rare)
};

class Settings {
public:
    static Settings& getInstance() {
        static Settings instance;
        return instance;
    }

    // Colorblind mode settings
    void setColorblindMode(ColorblindMode mode) {
        _colorblindMode = mode;
        // Reset shader to force recompilation with new mode
        _colorblindShader.reset();
    }
    ColorblindMode getColorblindMode() const { return _colorblindMode; }

    // Get colorblind mode name for display
    std::string getColorblindModeName() const {
        switch (_colorblindMode) {
            case ColorblindMode::None: return "None";
            case ColorblindMode::Protanopia: return "Protanopia";
            case ColorblindMode::Deuteranopia: return "Deuteranopia";
            case ColorblindMode::Tritanopia: return "Tritanopia";
            default: return "None";
        }
    }

    // Utility function to apply colorblind filter to a render::Color (CPU fallback)
    render::Color applyColorblindFilter(const render::Color& originalColor) const {
        if (_colorblindMode == ColorblindMode::None) {
            return originalColor;
        }

        // Convert to normalized values
        float r = originalColor.r / 255.0f;
        float g = originalColor.g / 255.0f;
        float b = originalColor.b / 255.0f;

        float newR = r, newG = g, newB = b;

        // Apply colorblind transformation matrices
        // Based on Brettel, Viénot and Mollon JPEG algorithm
        switch (_colorblindMode) {
            case ColorblindMode::Protanopia: // Red-blind
                newR = 0.567f * r + 0.433f * g;
                newG = 0.558f * r + 0.442f * g;
                newB = 0.242f * g + 0.758f * b;
                break;
            case ColorblindMode::Deuteranopia: // Green-blind
                newR = 0.625f * r + 0.375f * g;
                newG = 0.700f * r + 0.300f * g;
                newB = 0.300f * g + 0.700f * b;
                break;
            case ColorblindMode::Tritanopia: // Blue-blind
                newR = 0.950f * r + 0.050f * g;
                newG = 0.433f * g + 0.567f * b;
                newB = 0.475f * g + 0.525f * b;
                break;
            default:
                break;
        }

        // Clamp values to [0, 1]
        newR = std::max(0.0f, std::min(1.0f, newR));
        newG = std::max(0.0f, std::min(1.0f, newG));
        newB = std::max(0.0f, std::min(1.0f, newB));

        // Convert back to 0-255 range
        return render::Color(
            static_cast<uint8_t>(newR * 255),
            static_cast<uint8_t>(newG * 255),
            static_cast<uint8_t>(newB * 255),
            originalColor.a  // Keep alpha unchanged
        );
    }

    // Get colorblind shader for GPU acceleration
    render::IShader* getColorblindShader(render::IRenderWindow& window) {
        if (_colorblindMode == ColorblindMode::None) {
            return nullptr; // No shader needed
        }

        if (!_colorblindShader) {
            _colorblindShader = window.createShader();

            // Fragment shader for colorblind simulation
            const std::string fragmentShaderSource = R"(
                uniform sampler2D texture;
                uniform int mode; // 0=None, 1=Protanopia, 2=Deuteranopia, 3=Tritanopia

                void main() {
                    vec4 color = texture2D(texture, gl_TexCoord[0].xy);
                    vec3 rgb = color.rgb;
                    vec3 result = rgb;

                    if (mode == 1) {
                        // Protanopia (red-blind)
                        result.r = 0.567 * rgb.r + 0.433 * rgb.g;
                        result.g = 0.558 * rgb.r + 0.442 * rgb.g;
                        result.b = 0.242 * rgb.g + 0.758 * rgb.b;
                    } else if (mode == 2) {
                        // Deuteranopia (green-blind)
                        result.r = 0.625 * rgb.r + 0.375 * rgb.g;
                        result.g = 0.700 * rgb.r + 0.300 * rgb.g;
                        result.b = 0.300 * rgb.g + 0.700 * rgb.b;
                    } else if (mode == 3) {
                        // Tritanopia (blue-blind)
                        result.r = 0.950 * rgb.r + 0.050 * rgb.g;
                        result.g = 0.433 * rgb.g + 0.567 * rgb.b;
                        result.b = 0.475 * rgb.g + 0.525 * rgb.b;
                    }

                    gl_FragColor = vec4(result, color.a);
                }
            )";

            if (_colorblindShader->loadFromMemory(fragmentShaderSource, render::ShaderType::Fragment)) {
                _colorblindShader->setUniform("texture", 0);
            } else {
                // Fallback: shader loading failed
                _colorblindShader.reset();
            }
        }

        if (_colorblindShader) {
            _colorblindShader->setUniform("mode", static_cast<int>(_colorblindMode));
        }

        return _colorblindShader.get();
    }

    // Check if colorblind shader is available
    bool hasColorblindShader() const {
        return _colorblindShader != nullptr;
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
    Settings() : _colorblindMode(ColorblindMode::None), _soundEnabled(true), _resolutionWidth(800), _resolutionHeight(600) {}

    ColorblindMode _colorblindMode;
    bool _soundEnabled;
    unsigned int _resolutionWidth;
    unsigned int _resolutionHeight;
    std::unique_ptr<render::IShader> _colorblindShader;

    // Delete copy constructor and assignment operator
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;
};