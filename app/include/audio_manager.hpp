#pragma once
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <string>

enum class MusicType {
    MAIN_MENU,
    IN_GAME,
    GAME_OVER
};

class AudioManager {
public:
    AudioManager();
    ~AudioManager() = default;

    bool loadMusic(MusicType type, const std::string& filename);
    void playMusic(MusicType type, bool loop = true);
    void stopMusic();
    void pauseMusic();
    void resumeMusic();
    void setMusicVolume(float volume);
    bool isMusicPlaying() const;

private:
    std::unordered_map<MusicType, sf::Music> _musicTracks;
    MusicType _currentMusic = MusicType::MAIN_MENU;
    float _musicVolume = 50.0f;
};