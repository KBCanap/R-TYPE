/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** audio_manager
*/

#pragma once
#include "render/IRenderAudio.hpp"
#include <memory>
#include <string>
#include <unordered_map>

enum class MusicType { MAIN_MENU, IN_GAME, GAME_OVER };

class AudioManager {
  public:
    AudioManager(render::IRenderAudio &audioSystem);
    ~AudioManager() = default;

    bool loadMusic(MusicType type, const std::string &filename);
    void playMusic(MusicType type, bool loop = true);
    void stopMusic();
    void pauseMusic();
    void resumeMusic();
    void setMusicVolume(float volume);
    void setMasterVolume(float volume);
    bool isMusicPlaying() const;

    render::IRenderAudio &getAudioSystem() { return _audioSystem; }

  private:
    render::IRenderAudio &_audioSystem;
    std::unordered_map<MusicType, std::unique_ptr<render::IMusic>> _musicTracks;
    MusicType _currentMusic = MusicType::MAIN_MENU;
    float _musicVolume = 50.0f;
};