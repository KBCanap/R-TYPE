#pragma once
#include "../../../../ecs/include/render/IRenderAudio.hpp"
#include <SFML/Audio.hpp>
#include <memory>

namespace render {
namespace sfml {

// SFML Sound wrapper
class SFMLSound : public ISound {
public:
    SFMLSound() = default;
    void play() override;
    void pause() override;
    void stop() override;
    void setVolume(float volume) override;
    void setLoop(bool loop) override;
    void setPitch(float pitch) override;
    float getVolume() const override;
    bool getLoop() const override;
    AudioStatus getStatus() const override;
    void setBuffer(sf::SoundBuffer& buffer);

private:
    sf::Sound _sound;
    AudioStatus fromSFMLStatus(sf::Sound::Status status) const;
};

// SFML SoundBuffer wrapper
class SFMLSoundBuffer : public ISoundBuffer {
public:
    SFMLSoundBuffer() = default;
    bool loadFromFile(const std::string& filename) override;
    float getDuration() const override;
    sf::SoundBuffer& getNativeBuffer() { return _buffer; }

private:
    sf::SoundBuffer _buffer;
};

// SFML Music wrapper
class SFMLMusic : public IMusic {
public:
    SFMLMusic() = default;
    bool openFromFile(const std::string& filename) override;
    void play() override;
    void pause() override;
    void stop() override;
    void setVolume(float volume) override;
    void setLoop(bool loop) override;
    void setPitch(float pitch) override;
    void setPlayingOffset(float seconds) override;
    float getVolume() const override;
    bool getLoop() const override;
    AudioStatus getStatus() const override;
    float getDuration() const override;
    float getPlayingOffset() const override;

private:
    sf::Music _music;
    AudioStatus fromSFMLStatus(sf::Music::Status status) const;
};

// SFML RenderAudio implementation
class SFMLRenderAudio : public IRenderAudio {
public:
    SFMLRenderAudio() = default;
    ~SFMLRenderAudio() override = default;

    ISound* createSound() override;
    ISoundBuffer* createSoundBuffer() override;
    IMusic* createMusic() override;
    void setGlobalVolume(float volume) override;
    float getGlobalVolume() const override;

private:
    float _globalVolume = 100.0f;
};

} // namespace sfml
} // namespace render
