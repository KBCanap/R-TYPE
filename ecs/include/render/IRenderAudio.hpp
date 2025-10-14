/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** IRenderAudio
*/

#pragma once
#include <string>

namespace render {

enum class AudioStatus { Stopped, Paused, Playing };

// Forward declaration
class ISoundBuffer;

class ISound {
  public:
    virtual ~ISound() = default;
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    virtual void setVolume(float volume) = 0;
    virtual void setLoop(bool loop) = 0;
    virtual void setPitch(float pitch) = 0;
    virtual void setBuffer(ISoundBuffer &buffer) = 0;
    virtual float getVolume() const = 0;
    virtual bool getLoop() const = 0;
    virtual AudioStatus getStatus() const = 0;
};

class ISoundBuffer {
  public:
    virtual ~ISoundBuffer() = default;
    virtual bool loadFromFile(const std::string &filename) = 0;
    virtual float getDuration() const = 0;
};

class IMusic {
  public:
    virtual ~IMusic() = default;
    virtual bool openFromFile(const std::string &filename) = 0;
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    virtual void setVolume(float volume) = 0;
    virtual void setLoop(bool loop) = 0;
    virtual void setPitch(float pitch) = 0;
    virtual void setPlayingOffset(float seconds) = 0;
    virtual float getVolume() const = 0;
    virtual bool getLoop() const = 0;
    virtual AudioStatus getStatus() const = 0;
    virtual float getDuration() const = 0;
    virtual float getPlayingOffset() const = 0;
};

class IRenderAudio {
  public:
    virtual ~IRenderAudio() = default;

    // Factory methods
    virtual ISound *createSound() = 0;
    virtual ISoundBuffer *createSoundBuffer() = 0;
    virtual IMusic *createMusic() = 0;

    // Global audio settings
    virtual void setGlobalVolume(float volume) = 0;
    virtual float getGlobalVolume() const = 0;
};

} // namespace render
