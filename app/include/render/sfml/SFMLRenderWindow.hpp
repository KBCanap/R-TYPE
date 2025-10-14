/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** SFMLRenderWindow
*/

#pragma once
#include "../../../../ecs/include/render/IRenderWindow.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <unordered_map>

namespace render {
namespace sfml {

// SFML Image wrapper
class SFMLImage : public IImage {
  public:
    SFMLImage() = default;
    void create(unsigned int width, unsigned int height,
                const Color &color) override;
    sf::Image &getNativeImage() { return _image; }

  private:
    sf::Image _image;
};

// SFML Texture wrapper
class SFMLTexture : public ITexture {
  public:
    SFMLTexture() = default;
    bool loadFromFile(const std::string &filename) override;
    bool loadFromImage(IImage &image) override;
    Vector2u getSize() const override;
    void setSmooth(bool smooth) override;
    sf::Texture &getNativeTexture() { return _texture; }

  private:
    sf::Texture _texture;
};

// SFML Shader wrapper
class SFMLShader : public IShader {
  public:
    SFMLShader() = default;
    bool loadFromMemory(const std::string &shader, ShaderType type) override;
    void setUniform(const std::string &name, float value) override;
    void setUniform(const std::string &name, int value) override;
    sf::Shader &getNativeShader() { return _shader; }

  private:
    sf::Shader _shader;
};

// SFML Sprite wrapper
class SFMLSprite : public ISprite {
  public:
    SFMLSprite() = default;
    void setTexture(ITexture &texture) override;
    void setTextureRect(const IntRect &rect) override;
    void setPosition(float x, float y) override;
    void setPosition(const Vector2f &position) override;
    void setScale(float x, float y) override;
    void setScale(const Vector2f &scale) override;
    void setOrigin(float x, float y) override;
    void setOrigin(const Vector2f &origin) override;
    void setColor(const Color &color) override;
    void setRotation(float angle) override;
    Vector2f getPosition() const override;
    Vector2f getScale() const override;
    FloatRect getGlobalBounds() const override;
    sf::Sprite &getNativeSprite() { return _sprite; }

  private:
    sf::Sprite _sprite;
};

// SFML Shape wrapper
class SFMLShape : public IShape {
  public:
    explicit SFMLShape(std::unique_ptr<sf::Shape> shape)
        : _shape(std::move(shape)) {}
    void setPosition(float x, float y) override;
    void setPosition(const Vector2f &position) override;
    void setFillColor(const Color &color) override;
    void setOutlineColor(const Color &color) override;
    void setOutlineThickness(float thickness) override;
    FloatRect getGlobalBounds() const override;
    sf::Shape &getNativeShape() { return *_shape; }

  private:
    std::unique_ptr<sf::Shape> _shape;
};

// SFML Font wrapper
class SFMLFont : public IFont {
  public:
    SFMLFont() = default;
    bool loadFromFile(const std::string &filename) override;
    sf::Font &getNativeFont() { return _font; }

  private:
    sf::Font _font;
};

// SFML Text wrapper
class SFMLText : public IText {
  public:
    SFMLText() = default;
    void setFont(IFont &font) override;
    void setString(const std::string &string) override;
    void setCharacterSize(unsigned int size) override;
    void setFillColor(const Color &color) override;
    void setOutlineColor(const Color &color) override;
    void setOutlineThickness(float thickness) override;
    void setPosition(float x, float y) override;
    void setPosition(const Vector2f &position) override;
    void setStyle(uint32_t style) override;
    FloatRect getGlobalBounds() const override;
    FloatRect getLocalBounds() const override;
    sf::Text &getNativeText() { return _text; }

  private:
    sf::Text _text;
};

// SFML View wrapper
class SFMLView : public IView {
  public:
    SFMLView() = default;
    explicit SFMLView(const sf::View &view) : _view(view) {}
    void reset(const FloatRect &rectangle) override;
    void setSize(float width, float height) override;
    void setSize(const Vector2f &size) override;
    void setCenter(float x, float y) override;
    void setCenter(const Vector2f &center) override;
    Vector2f getSize() const override;
    Vector2f getCenter() const override;
    sf::View &getNativeView() { return _view; }
    const sf::View &getNativeView() const { return _view; }

  private:
    sf::View _view;
};

// SFML RenderWindow wrapper
class SFMLRenderWindow : public IRenderWindow {
  public:
    SFMLRenderWindow(unsigned int width, unsigned int height,
                     const std::string &title);
    ~SFMLRenderWindow() override = default;

    // Window management
    bool isOpen() const override;
    void close() override;
    void clear(const Color &color = Color::Black()) override;
    void display() override;
    Vector2u getSize() const override;
    void setSize(const Vector2u &size) override;
    void setFramerateLimit(unsigned int limit) override;
    void setVerticalSyncEnabled(bool enabled) override;
    void setTitle(const std::string &title) override;

    // Event handling
    bool pollEvent(Event &event) override;

    // Drawing
    void draw(ISprite &sprite) override;
    void draw(IShape &shape) override;
    void draw(IText &text) override;
    void draw(ISprite &sprite, IShader &shader) override;
    void draw(IShape &shape, IShader &shader) override;
    void draw(IText &text, IShader &shader) override;

    // View management
    void setView(IView &view) override;
    std::unique_ptr<IView> getDefaultView() const override;
    std::unique_ptr<IView> createView() override;

    // Factory methods
    std::unique_ptr<ISprite> createSprite() override;
    std::unique_ptr<ITexture> createTexture() override;
    std::unique_ptr<IShape> createRectangleShape(const Vector2f &size) override;
    std::unique_ptr<IShape> createCircleShape(float radius) override;
    std::unique_ptr<IFont> createFont() override;
    std::unique_ptr<IText> createText() override;
    std::unique_ptr<IShader> createShader() override;
    std::unique_ptr<IImage> createImage() override;

    // Access to native window (for compatibility during transition)
    sf::RenderWindow &getNativeWindow() { return _window; }

    // Event conversion for compatibility with non-migrated components
    static sf::Event toSFMLEvent(const Event &event);

  private:
    sf::RenderWindow _window;

    // Conversion helpers
    sf::Color toSFMLColor(const Color &color) const;
    Color fromSFMLColor(const sf::Color &color) const;
    Key fromSFMLKey(sf::Keyboard::Key key) const;
    Mouse fromSFMLMouse(sf::Mouse::Button button) const;
    static sf::Keyboard::Key toSFMLKey(Key key);
    static sf::Mouse::Button toSFMLMouse(Mouse button);
};

} // namespace sfml
} // namespace render
