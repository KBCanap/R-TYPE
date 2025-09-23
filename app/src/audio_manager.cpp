#include "../include/audio_manager.hpp"
#include <iostream>

AudioManager::AudioManager() {}

bool AudioManager::loadMusic(MusicType type, const std::string& filename) {
    if (!_musicTracks[type].openFromFile(filename)) {
        std::cerr << "Error: Could not load music file: " << filename << std::endl;
        return false;
    }
    _musicTracks[type].setVolume(_musicVolume);
    return true;
}

void AudioManager::playMusic(MusicType type, bool loop) {
    stopMusic();

    auto it = _musicTracks.find(type);
    if (it != _musicTracks.end()) {
        _currentMusic = type;
        it->second.setLoop(loop);
        it->second.play();
    }
}

void AudioManager::stopMusic() {
    for (auto& [type, music] : _musicTracks) {
        if (music.getStatus() == sf::Music::Playing) {
            music.stop();
        }
    }
}

void AudioManager::pauseMusic() {
    auto it = _musicTracks.find(_currentMusic);
    if (it != _musicTracks.end() && it->second.getStatus() == sf::Music::Playing) {
        it->second.pause();
    }
}

void AudioManager::resumeMusic() {
    auto it = _musicTracks.find(_currentMusic);
    if (it != _musicTracks.end() && it->second.getStatus() == sf::Music::Paused) {
        it->second.play();
    }
}

void AudioManager::setMusicVolume(float volume) {
    _musicVolume = volume;
    for (auto& [type, music] : _musicTracks) {
        music.setVolume(_musicVolume);
    }
}

bool AudioManager::isMusicPlaying() const {
    auto it = _musicTracks.find(_currentMusic);
    return it != _musicTracks.end() && it->second.getStatus() == sf::Music::Playing;
}