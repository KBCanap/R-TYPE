#include "render/sfml/SFMLRenderAudio.hpp"

namespace render {
namespace sfml {

void SFMLSound::play() { _sound.play(); }

void SFMLSound::pause() { _sound.pause(); }

void SFMLSound::stop() { _sound.stop(); }

void SFMLSound::setVolume(float volume) { _sound.setVolume(volume); }

void SFMLSound::setLoop(bool loop) { _sound.setLoop(loop); }

void SFMLSound::setPitch(float pitch) { _sound.setPitch(pitch); }

float SFMLSound::getVolume() const { return _sound.getVolume(); }

bool SFMLSound::getLoop() const { return _sound.getLoop(); }

AudioStatus SFMLSound::getStatus() const {
    return fromSFMLStatus(_sound.getStatus());
}

void SFMLSound::setBuffer(ISoundBuffer &buffer) {
    auto &sfmlBuffer = dynamic_cast<SFMLSoundBuffer &>(buffer);
    _sound.setBuffer(sfmlBuffer.getNativeBuffer());
}

AudioStatus SFMLSound::fromSFMLStatus(sf::Sound::Status status) const {
    switch (status) {
    case sf::Sound::Stopped:
        return AudioStatus::Stopped;
    case sf::Sound::Paused:
        return AudioStatus::Paused;
    case sf::Sound::Playing:
        return AudioStatus::Playing;
    default:
        return AudioStatus::Stopped;
    }
}

bool SFMLSoundBuffer::loadFromFile(const std::string &filename) {
    return _buffer.loadFromFile(filename);
}

float SFMLSoundBuffer::getDuration() const {
    return _buffer.getDuration().asSeconds();
}

bool SFMLMusic::openFromFile(const std::string &filename) {
    return _music.openFromFile(filename);
}

void SFMLMusic::play() { _music.play(); }

void SFMLMusic::pause() { _music.pause(); }

void SFMLMusic::stop() { _music.stop(); }

void SFMLMusic::setVolume(float volume) { _music.setVolume(volume); }

void SFMLMusic::setLoop(bool loop) { _music.setLoop(loop); }

void SFMLMusic::setPitch(float pitch) { _music.setPitch(pitch); }

void SFMLMusic::setPlayingOffset(float seconds) {
    _music.setPlayingOffset(sf::seconds(seconds));
}

float SFMLMusic::getVolume() const { return _music.getVolume(); }

bool SFMLMusic::getLoop() const { return _music.getLoop(); }

AudioStatus SFMLMusic::getStatus() const {
    return fromSFMLStatus(_music.getStatus());
}

float SFMLMusic::getDuration() const {
    return _music.getDuration().asSeconds();
}

float SFMLMusic::getPlayingOffset() const {
    return _music.getPlayingOffset().asSeconds();
}

AudioStatus SFMLMusic::fromSFMLStatus(sf::Music::Status status) const {
    switch (status) {
    case sf::Music::Stopped:
        return AudioStatus::Stopped;
    case sf::Music::Paused:
        return AudioStatus::Paused;
    case sf::Music::Playing:
        return AudioStatus::Playing;
    default:
        return AudioStatus::Stopped;
    }
}

ISound *SFMLRenderAudio::createSound() { return new SFMLSound(); }

ISoundBuffer *SFMLRenderAudio::createSoundBuffer() {
    return new SFMLSoundBuffer();
}

IMusic *SFMLRenderAudio::createMusic() { return new SFMLMusic(); }

void SFMLRenderAudio::setGlobalVolume(float volume) {
    _globalVolume = volume;
    sf::Listener::setGlobalVolume(volume);
}

float SFMLRenderAudio::getGlobalVolume() const { return _globalVolume; }

} // namespace sfml
} // namespace render
