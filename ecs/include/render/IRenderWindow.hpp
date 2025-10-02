#pragma once
#include <string>
#include <cstdint>
#include <memory>

namespace render {

struct Color {
    uint8_t r, g, b, a;
    Color(uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t a = 255)
        : r(r), g(g), b(b), a(a) {}

    static Color White() { return Color(255, 255, 255, 255); }
    static Color Black() { return Color(0, 0, 0, 255); }
    static Color Red() { return Color(255, 0, 0, 255); }
    static Color Green() { return Color(0, 255, 0, 255); }
    static Color Blue() { return Color(0, 0, 255, 255); }
    static Color Yellow() { return Color(255, 255, 0, 255); }
    static Color Cyan() { return Color(0, 255, 255, 255); }
    static Color Magenta() { return Color(255, 0, 255, 255); }
};

struct IntRect {
    int left, top, width, height;
    IntRect(int left = 0, int top = 0, int width = 0, int height = 0)
        : left(left), top(top), width(width), height(height) {}
};

struct FloatRect {
    float left, top, width, height;
    FloatRect(float left = 0, float top = 0, float width = 0, float height = 0)
        : left(left), top(top), width(width), height(height) {}
};

struct Vector2f {
    float x, y;
    Vector2f(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}
};

struct Vector2i {
    int x, y;
    Vector2i(int x = 0, int y = 0) : x(x), y(y) {}
};

struct Vector2u {
    unsigned int x, y;
    Vector2u(unsigned int x = 0, unsigned int y = 0) : x(x), y(y) {}
};

enum class EventType {
    Closed,
    KeyPressed,
    KeyReleased,
    MouseButtonPressed,
    MouseButtonReleased,
    MouseMoved,
    Resized,
    Unknown
};

enum class Key {
    Unknown = -1,
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    Escape, LControl, LShift, LAlt, LSystem,
    RControl, RShift, RAlt, RSystem,
    Menu, LBracket, RBracket, Semicolon, Comma, Period,
    Quote, Slash, Backslash, Tilde, Equal, Hyphen,
    Space, Enter, Backspace, Tab,
    PageUp, PageDown, End, Home, Insert, Delete,
    Add, Subtract, Multiply, Divide,
    Left, Right, Up, Down,
    Numpad0, Numpad1, Numpad2, Numpad3, Numpad4,
    Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12
};

enum class Mouse {
    Left,
    Right,
    Middle,
    XButton1,
    XButton2
};

struct Event {
    EventType type;

    struct KeyEvent {
        Key code;
        bool alt;
        bool control;
        bool shift;
        bool system;
    };

    struct MouseButtonEvent {
        Mouse button;
        int x;
        int y;
    };

    struct MouseMoveEvent {
        int x;
        int y;
    };

    struct SizeEvent {
        unsigned int width;
        unsigned int height;
    };

    union {
        KeyEvent key;
        MouseButtonEvent mouseButton;
        MouseMoveEvent mouseMove;
        SizeEvent size;
    };
};

class IImage {
public:
    virtual ~IImage() = default;
    virtual void create(unsigned int width, unsigned int height, const Color& color) = 0;
};

class ITexture {
public:
    virtual ~ITexture() = default;
    virtual bool loadFromFile(const std::string& filename) = 0;
    virtual bool loadFromImage(IImage& image) = 0;
    virtual Vector2u getSize() const = 0;
    virtual void setSmooth(bool smooth) = 0;
};

enum class ShaderType {
    Vertex,
    Fragment
};

class IShader {
public:
    virtual ~IShader() = default;
    virtual bool loadFromMemory(const std::string& shader, ShaderType type) = 0;
    virtual void setUniform(const std::string& name, float value) = 0;
    virtual void setUniform(const std::string& name, int value) = 0;
};

class ISprite {
public:
    virtual ~ISprite() = default;
    virtual void setTexture(ITexture& texture) = 0;
    virtual void setTextureRect(const IntRect& rect) = 0;
    virtual void setPosition(float x, float y) = 0;
    virtual void setPosition(const Vector2f& position) = 0;
    virtual void setScale(float x, float y) = 0;
    virtual void setScale(const Vector2f& scale) = 0;
    virtual void setOrigin(float x, float y) = 0;
    virtual void setOrigin(const Vector2f& origin) = 0;
    virtual void setColor(const Color& color) = 0;
    virtual void setRotation(float angle) = 0;
    virtual Vector2f getPosition() const = 0;
    virtual Vector2f getScale() const = 0;
    virtual FloatRect getGlobalBounds() const = 0;
};

class IShape {
public:
    virtual ~IShape() = default;
    virtual void setPosition(float x, float y) = 0;
    virtual void setPosition(const Vector2f& position) = 0;
    virtual void setFillColor(const Color& color) = 0;
    virtual void setOutlineColor(const Color& color) = 0;
    virtual void setOutlineThickness(float thickness) = 0;
    virtual FloatRect getGlobalBounds() const = 0;
};

class IFont {
public:
    virtual ~IFont() = default;
    virtual bool loadFromFile(const std::string& filename) = 0;
};

class IText {
public:
    virtual ~IText() = default;
    virtual void setFont(IFont& font) = 0;
    virtual void setString(const std::string& string) = 0;
    virtual void setCharacterSize(unsigned int size) = 0;
    virtual void setFillColor(const Color& color) = 0;
    virtual void setOutlineColor(const Color& color) = 0;
    virtual void setOutlineThickness(float thickness) = 0;
    virtual void setPosition(float x, float y) = 0;
    virtual void setPosition(const Vector2f& position) = 0;
    virtual void setStyle(uint32_t style) = 0;
    virtual FloatRect getGlobalBounds() const = 0;
    virtual FloatRect getLocalBounds() const = 0;
};

class IView {
public:
    virtual ~IView() = default;
    virtual void reset(const FloatRect& rectangle) = 0;
    virtual void setSize(float width, float height) = 0;
    virtual void setSize(const Vector2f& size) = 0;
    virtual void setCenter(float x, float y) = 0;
    virtual void setCenter(const Vector2f& center) = 0;
    virtual Vector2f getSize() const = 0;
    virtual Vector2f getCenter() const = 0;
};

class IRenderWindow {
public:
    virtual ~IRenderWindow() = default;

    // Window management
    virtual bool isOpen() const = 0;
    virtual void close() = 0;
    virtual void clear(const Color& color = Color::Black()) = 0;
    virtual void display() = 0;
    virtual Vector2u getSize() const = 0;
    virtual void setSize(const Vector2u& size) = 0;
    virtual void setFramerateLimit(unsigned int limit) = 0;
    virtual void setVerticalSyncEnabled(bool enabled) = 0;
    virtual void setTitle(const std::string& title) = 0;

    // Event handling
    virtual bool pollEvent(Event& event) = 0;

    // Drawing
    virtual void draw(ISprite& sprite) = 0;
    virtual void draw(IShape& shape) = 0;
    virtual void draw(IText& text) = 0;
    virtual void draw(ISprite& sprite, IShader& shader) = 0;
    virtual void draw(IShape& shape, IShader& shader) = 0;
    virtual void draw(IText& text, IShader& shader) = 0;

    // View management
    virtual void setView(IView& view) = 0;
    virtual std::unique_ptr<IView> getDefaultView() const = 0;
    virtual std::unique_ptr<IView> createView() = 0;

    // Factory methods for creating drawable objects
    virtual std::unique_ptr<ISprite> createSprite() = 0;
    virtual std::unique_ptr<ITexture> createTexture() = 0;
    virtual std::unique_ptr<IShape> createRectangleShape(const Vector2f& size) = 0;
    virtual std::unique_ptr<IShape> createCircleShape(float radius) = 0;
    virtual std::unique_ptr<IFont> createFont() = 0;
    virtual std::unique_ptr<IText> createText() = 0;
    virtual std::unique_ptr<IShader> createShader() = 0;
    virtual std::unique_ptr<IImage> createImage() = 0;
};

} // namespace render
