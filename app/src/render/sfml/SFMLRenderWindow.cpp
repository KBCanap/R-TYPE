#include "render/sfml/SFMLRenderWindow.hpp"
#include <SFML/Graphics.hpp>

namespace render {
namespace sfml {

// SFMLTexture implementation
bool SFMLTexture::loadFromFile(const std::string& filename) {
    return _texture.loadFromFile(filename);
}

Vector2u SFMLTexture::getSize() const {
    auto size = _texture.getSize();
    return Vector2u(size.x, size.y);
}

void SFMLTexture::setSmooth(bool smooth) {
    _texture.setSmooth(smooth);
}

// SFMLSprite implementation
void SFMLSprite::setTexture(ITexture& texture) {
    auto& sfmlTexture = dynamic_cast<SFMLTexture&>(texture);
    _sprite.setTexture(sfmlTexture.getNativeTexture());
}

void SFMLSprite::setTextureRect(const IntRect& rect) {
    _sprite.setTextureRect(sf::IntRect(rect.left, rect.top, rect.width, rect.height));
}

void SFMLSprite::setPosition(float x, float y) {
    _sprite.setPosition(x, y);
}

void SFMLSprite::setPosition(const Vector2f& position) {
    _sprite.setPosition(position.x, position.y);
}

void SFMLSprite::setScale(float x, float y) {
    _sprite.setScale(x, y);
}

void SFMLSprite::setScale(const Vector2f& scale) {
    _sprite.setScale(scale.x, scale.y);
}

void SFMLSprite::setOrigin(float x, float y) {
    _sprite.setOrigin(x, y);
}

void SFMLSprite::setOrigin(const Vector2f& origin) {
    _sprite.setOrigin(origin.x, origin.y);
}

void SFMLSprite::setColor(const Color& color) {
    _sprite.setColor(sf::Color(color.r, color.g, color.b, color.a));
}

void SFMLSprite::setRotation(float angle) {
    _sprite.setRotation(angle);
}

Vector2f SFMLSprite::getPosition() const {
    auto pos = _sprite.getPosition();
    return Vector2f(pos.x, pos.y);
}

Vector2f SFMLSprite::getScale() const {
    auto scale = _sprite.getScale();
    return Vector2f(scale.x, scale.y);
}

FloatRect SFMLSprite::getGlobalBounds() const {
    auto bounds = _sprite.getGlobalBounds();
    return FloatRect(bounds.left, bounds.top, bounds.width, bounds.height);
}

// SFMLShape implementation
void SFMLShape::setPosition(float x, float y) {
    _shape->setPosition(x, y);
}

void SFMLShape::setPosition(const Vector2f& position) {
    _shape->setPosition(position.x, position.y);
}

void SFMLShape::setFillColor(const Color& color) {
    _shape->setFillColor(sf::Color(color.r, color.g, color.b, color.a));
}

void SFMLShape::setOutlineColor(const Color& color) {
    _shape->setOutlineColor(sf::Color(color.r, color.g, color.b, color.a));
}

void SFMLShape::setOutlineThickness(float thickness) {
    _shape->setOutlineThickness(thickness);
}

FloatRect SFMLShape::getGlobalBounds() const {
    auto bounds = _shape->getGlobalBounds();
    return FloatRect(bounds.left, bounds.top, bounds.width, bounds.height);
}

// SFMLFont implementation
bool SFMLFont::loadFromFile(const std::string& filename) {
    return _font.loadFromFile(filename);
}

// SFMLText implementation
void SFMLText::setFont(IFont& font) {
    auto& sfmlFont = dynamic_cast<SFMLFont&>(font);
    _text.setFont(sfmlFont.getNativeFont());
}

void SFMLText::setString(const std::string& string) {
    _text.setString(string);
}

void SFMLText::setCharacterSize(unsigned int size) {
    _text.setCharacterSize(size);
}

void SFMLText::setFillColor(const Color& color) {
    _text.setFillColor(sf::Color(color.r, color.g, color.b, color.a));
}

void SFMLText::setOutlineColor(const Color& color) {
    _text.setOutlineColor(sf::Color(color.r, color.g, color.b, color.a));
}

void SFMLText::setOutlineThickness(float thickness) {
    _text.setOutlineThickness(thickness);
}

void SFMLText::setPosition(float x, float y) {
    _text.setPosition(x, y);
}

void SFMLText::setPosition(const Vector2f& position) {
    _text.setPosition(position.x, position.y);
}

void SFMLText::setStyle(uint32_t style) {
    _text.setStyle(style);
}

FloatRect SFMLText::getGlobalBounds() const {
    auto bounds = _text.getGlobalBounds();
    return FloatRect(bounds.left, bounds.top, bounds.width, bounds.height);
}

FloatRect SFMLText::getLocalBounds() const {
    auto bounds = _text.getLocalBounds();
    return FloatRect(bounds.left, bounds.top, bounds.width, bounds.height);
}

// SFMLRenderWindow implementation
SFMLRenderWindow::SFMLRenderWindow(unsigned int width, unsigned int height, const std::string& title)
    : _window(sf::VideoMode(width, height), title) {
}

bool SFMLRenderWindow::isOpen() const {
    return _window.isOpen();
}

void SFMLRenderWindow::close() {
    _window.close();
}

void SFMLRenderWindow::clear(const Color& color) {
    _window.clear(toSFMLColor(color));
}

void SFMLRenderWindow::display() {
    _window.display();
}

Vector2u SFMLRenderWindow::getSize() const {
    auto size = _window.getSize();
    return Vector2u(size.x, size.y);
}

void SFMLRenderWindow::setFramerateLimit(unsigned int limit) {
    _window.setFramerateLimit(limit);
}

void SFMLRenderWindow::setVerticalSyncEnabled(bool enabled) {
    _window.setVerticalSyncEnabled(enabled);
}

void SFMLRenderWindow::setTitle(const std::string& title) {
    _window.setTitle(title);
}

bool SFMLRenderWindow::pollEvent(Event& event) {
    sf::Event sfEvent;
    if (!_window.pollEvent(sfEvent)) {
        return false;
    }

    // Convert SFML event to generic event
    switch (sfEvent.type) {
        case sf::Event::Closed:
            event.type = EventType::Closed;
            break;
        case sf::Event::KeyPressed:
            event.type = EventType::KeyPressed;
            event.key.code = fromSFMLKey(sfEvent.key.code);
            event.key.alt = sfEvent.key.alt;
            event.key.control = sfEvent.key.control;
            event.key.shift = sfEvent.key.shift;
            event.key.system = sfEvent.key.system;
            break;
        case sf::Event::KeyReleased:
            event.type = EventType::KeyReleased;
            event.key.code = fromSFMLKey(sfEvent.key.code);
            event.key.alt = sfEvent.key.alt;
            event.key.control = sfEvent.key.control;
            event.key.shift = sfEvent.key.shift;
            event.key.system = sfEvent.key.system;
            break;
        case sf::Event::MouseButtonPressed:
            event.type = EventType::MouseButtonPressed;
            event.mouseButton.button = fromSFMLMouse(sfEvent.mouseButton.button);
            event.mouseButton.x = sfEvent.mouseButton.x;
            event.mouseButton.y = sfEvent.mouseButton.y;
            break;
        case sf::Event::MouseButtonReleased:
            event.type = EventType::MouseButtonReleased;
            event.mouseButton.button = fromSFMLMouse(sfEvent.mouseButton.button);
            event.mouseButton.x = sfEvent.mouseButton.x;
            event.mouseButton.y = sfEvent.mouseButton.y;
            break;
        case sf::Event::MouseMoved:
            event.type = EventType::MouseMoved;
            event.mouseMove.x = sfEvent.mouseMove.x;
            event.mouseMove.y = sfEvent.mouseMove.y;
            break;
        case sf::Event::Resized:
            event.type = EventType::Resized;
            event.size.width = sfEvent.size.width;
            event.size.height = sfEvent.size.height;
            break;
        default:
            event.type = EventType::Unknown;
            break;
    }

    return true;
}

void SFMLRenderWindow::draw(ISprite& sprite) {
    auto& sfmlSprite = dynamic_cast<SFMLSprite&>(sprite);
    _window.draw(sfmlSprite.getNativeSprite());
}

void SFMLRenderWindow::draw(IShape& shape) {
    auto& sfmlShape = dynamic_cast<SFMLShape&>(shape);
    _window.draw(sfmlShape.getNativeShape());
}

void SFMLRenderWindow::draw(IText& text) {
    auto& sfmlText = dynamic_cast<SFMLText&>(text);
    _window.draw(sfmlText.getNativeText());
}

ISprite* SFMLRenderWindow::createSprite() {
    return new SFMLSprite();
}

ITexture* SFMLRenderWindow::createTexture() {
    return new SFMLTexture();
}

IShape* SFMLRenderWindow::createRectangleShape(const Vector2f& size) {
    auto shape = std::make_unique<sf::RectangleShape>(sf::Vector2f(size.x, size.y));
    return new SFMLShape(std::move(shape));
}

IShape* SFMLRenderWindow::createCircleShape(float radius) {
    auto shape = std::make_unique<sf::CircleShape>(radius);
    return new SFMLShape(std::move(shape));
}

IFont* SFMLRenderWindow::createFont() {
    return new SFMLFont();
}

IText* SFMLRenderWindow::createText() {
    return new SFMLText();
}

// Helper methods
sf::Color SFMLRenderWindow::toSFMLColor(const Color& color) const {
    return sf::Color(color.r, color.g, color.b, color.a);
}

Color SFMLRenderWindow::fromSFMLColor(const sf::Color& color) const {
    return Color(color.r, color.g, color.b, color.a);
}

Key SFMLRenderWindow::fromSFMLKey(sf::Keyboard::Key key) const {
    // Mapping SFML keys to generic keys
    static const std::unordered_map<sf::Keyboard::Key, Key> keyMap = {
        {sf::Keyboard::Unknown, Key::Unknown},
        {sf::Keyboard::A, Key::A}, {sf::Keyboard::B, Key::B}, {sf::Keyboard::C, Key::C},
        {sf::Keyboard::D, Key::D}, {sf::Keyboard::E, Key::E}, {sf::Keyboard::F, Key::F},
        {sf::Keyboard::G, Key::G}, {sf::Keyboard::H, Key::H}, {sf::Keyboard::I, Key::I},
        {sf::Keyboard::J, Key::J}, {sf::Keyboard::K, Key::K}, {sf::Keyboard::L, Key::L},
        {sf::Keyboard::M, Key::M}, {sf::Keyboard::N, Key::N}, {sf::Keyboard::O, Key::O},
        {sf::Keyboard::P, Key::P}, {sf::Keyboard::Q, Key::Q}, {sf::Keyboard::R, Key::R},
        {sf::Keyboard::S, Key::S}, {sf::Keyboard::T, Key::T}, {sf::Keyboard::U, Key::U},
        {sf::Keyboard::V, Key::V}, {sf::Keyboard::W, Key::W}, {sf::Keyboard::X, Key::X},
        {sf::Keyboard::Y, Key::Y}, {sf::Keyboard::Z, Key::Z},
        {sf::Keyboard::Num0, Key::Num0}, {sf::Keyboard::Num1, Key::Num1},
        {sf::Keyboard::Num2, Key::Num2}, {sf::Keyboard::Num3, Key::Num3},
        {sf::Keyboard::Num4, Key::Num4}, {sf::Keyboard::Num5, Key::Num5},
        {sf::Keyboard::Num6, Key::Num6}, {sf::Keyboard::Num7, Key::Num7},
        {sf::Keyboard::Num8, Key::Num8}, {sf::Keyboard::Num9, Key::Num9},
        {sf::Keyboard::Escape, Key::Escape}, {sf::Keyboard::LControl, Key::LControl},
        {sf::Keyboard::LShift, Key::LShift}, {sf::Keyboard::LAlt, Key::LAlt},
        {sf::Keyboard::LSystem, Key::LSystem}, {sf::Keyboard::RControl, Key::RControl},
        {sf::Keyboard::RShift, Key::RShift}, {sf::Keyboard::RAlt, Key::RAlt},
        {sf::Keyboard::RSystem, Key::RSystem}, {sf::Keyboard::Menu, Key::Menu},
        {sf::Keyboard::LBracket, Key::LBracket}, {sf::Keyboard::RBracket, Key::RBracket},
        {sf::Keyboard::Semicolon, Key::Semicolon}, {sf::Keyboard::Comma, Key::Comma},
        {sf::Keyboard::Period, Key::Period}, {sf::Keyboard::Quote, Key::Quote},
        {sf::Keyboard::Slash, Key::Slash}, {sf::Keyboard::Backslash, Key::Backslash},
        {sf::Keyboard::Tilde, Key::Tilde}, {sf::Keyboard::Equal, Key::Equal},
        {sf::Keyboard::Hyphen, Key::Hyphen}, {sf::Keyboard::Space, Key::Space},
        {sf::Keyboard::Enter, Key::Enter}, {sf::Keyboard::Backspace, Key::Backspace},
        {sf::Keyboard::Tab, Key::Tab}, {sf::Keyboard::PageUp, Key::PageUp},
        {sf::Keyboard::PageDown, Key::PageDown}, {sf::Keyboard::End, Key::End},
        {sf::Keyboard::Home, Key::Home}, {sf::Keyboard::Insert, Key::Insert},
        {sf::Keyboard::Delete, Key::Delete}, {sf::Keyboard::Add, Key::Add},
        {sf::Keyboard::Subtract, Key::Subtract}, {sf::Keyboard::Multiply, Key::Multiply},
        {sf::Keyboard::Divide, Key::Divide}, {sf::Keyboard::Left, Key::Left},
        {sf::Keyboard::Right, Key::Right}, {sf::Keyboard::Up, Key::Up},
        {sf::Keyboard::Down, Key::Down}, {sf::Keyboard::Numpad0, Key::Numpad0},
        {sf::Keyboard::Numpad1, Key::Numpad1}, {sf::Keyboard::Numpad2, Key::Numpad2},
        {sf::Keyboard::Numpad3, Key::Numpad3}, {sf::Keyboard::Numpad4, Key::Numpad4},
        {sf::Keyboard::Numpad5, Key::Numpad5}, {sf::Keyboard::Numpad6, Key::Numpad6},
        {sf::Keyboard::Numpad7, Key::Numpad7}, {sf::Keyboard::Numpad8, Key::Numpad8},
        {sf::Keyboard::Numpad9, Key::Numpad9}, {sf::Keyboard::F1, Key::F1},
        {sf::Keyboard::F2, Key::F2}, {sf::Keyboard::F3, Key::F3}, {sf::Keyboard::F4, Key::F4},
        {sf::Keyboard::F5, Key::F5}, {sf::Keyboard::F6, Key::F6}, {sf::Keyboard::F7, Key::F7},
        {sf::Keyboard::F8, Key::F8}, {sf::Keyboard::F9, Key::F9}, {sf::Keyboard::F10, Key::F10},
        {sf::Keyboard::F11, Key::F11}, {sf::Keyboard::F12, Key::F12}
    };

    auto it = keyMap.find(key);
    return (it != keyMap.end()) ? it->second : Key::Unknown;
}

Mouse SFMLRenderWindow::fromSFMLMouse(sf::Mouse::Button button) const {
    switch (button) {
        case sf::Mouse::Left: return Mouse::Left;
        case sf::Mouse::Right: return Mouse::Right;
        case sf::Mouse::Middle: return Mouse::Middle;
        case sf::Mouse::XButton1: return Mouse::XButton1;
        case sf::Mouse::XButton2: return Mouse::XButton2;
        default: return Mouse::Left;
    }
}

} // namespace sfml
} // namespace render
