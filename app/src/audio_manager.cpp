#include "../include/audio_manager.hpp"
#include <iostream>

AudioManager::AudioManager(render::IRenderAudio& audioSystem)
    : _audioSystem(audioSystem) {}

bool AudioManager::loadMusic(MusicType type, const std::string& filename) {
    auto music = std::unique_ptr<render::IMusic>(_audioSystem.createMusic());
    if (!music->openFromFile(filename)) {
        std::cerr << "Error: Could not load music file: " << filename << std::endl;
        return false;
    }
    music->setVolume(_musicVolume);
    _musicTracks[type] = std::move(music);
    return true;
}

void AudioManager::playMusic(MusicType type, bool loop) {
    stopMusic();
    auto it = _musicTracks.find(type);
    if (it != _musicTracks.end()) {
        _currentMusic = type;
        it->second->setLoop(loop);
        it->second->play();
    }
}

void AudioManager::stopMusic() {
    for (auto& [type, music] : _musicTracks) {
        if (music->getStatus() == render::AudioStatus::Playing) {
            music->stop();
        }
    }
}

void AudioManager::pauseMusic() {
    auto it = _musicTracks.find(_currentMusic);
    if (it != _musicTracks.end() && it->second->getStatus() == render::AudioStatus::Playing) {
        it->second->pause();
    }
}

void AudioManager::resumeMusic() {
    auto it = _musicTracks.find(_currentMusic);
    if (it != _musicTracks.end() && it->second->getStatus() == render::AudioStatus::Paused) {
        it->second->play();
    }
}

void AudioManager::setMusicVolume(float volume) {
    _musicVolume = volume;
    for (auto& [type, music] : _musicTracks) {
        music->setVolume(_musicVolume);
    }
}

void AudioManager::setMasterVolume(float volume) {
    setMusicVolume(volume);
}

bool AudioManager::isMusicPlaying() const {
    auto it = _musicTracks.find(_currentMusic);
    return it != _musicTracks.end() && it->second->getStatus() == render::AudioStatus::Playing;
}