/*
** EPITECH PROJECT, 2025
** R-TYPE
** File description:
** audio_system
*/

#include "../../include/systems.hpp"
#include "../include/render/IRenderAudio.hpp"
#include <unordered_map>

namespace systems {

void audio_system(registry & /*r*/,
                  sparse_array<component::sound_effect> &sound_effects,
                  sparse_array<component::music> &musics,
                  sparse_array<component::audio_trigger> &triggers,
                  render::IRenderAudio &audioManager) {

    static std::unordered_map<std::string, render::ISoundBuffer *>
        sound_buffers;
    static std::unordered_map<std::string, render::ISound *> sounds;
    static std::unordered_map<std::string, render::IMusic *> music_tracks;

    for (size_t i = 0; i < std::min(sound_effects.size(), triggers.size());
         ++i) {
        auto &sound_effect = sound_effects[i];
        auto &trigger = triggers[i];

        bool should_trigger = sound_effect && trigger && !trigger->triggered;
        if (!should_trigger) continue;

        // Single map lookup instead of two
        auto buffer_it = sound_buffers.find(sound_effect->sound_path);
        bool buffer_exists = (buffer_it != sound_buffers.end());

        // Load buffer if needed (max 2 branches)
        if (!buffer_exists) {
            auto *buffer = audioManager.createSoundBuffer();
            bool loaded = buffer->loadFromFile(sound_effect->sound_path);
            if (loaded) {
                sound_buffers[sound_effect->sound_path] = buffer;
                auto *sound = audioManager.createSound();
                sound->setBuffer(*buffer);
                sounds[sound_effect->sound_path] = sound;
                buffer_exists = true;
            } else {
                delete buffer;
            }
        }

        // Play sound if buffer exists
        if (buffer_exists) {
            auto *sound = sounds[sound_effect->sound_path];
            sound->setVolume(sound_effect->volume);
            sound->play();
            sound_effect->is_playing = true;

            trigger->triggered = sound_effect->play_once;
        }

        // Update playing status (max 2 branches but separated)
        bool should_check_status = sound_effect && sound_effect->is_playing;
        if (should_check_status) {
            auto sound_it = sounds.find(sound_effect->sound_path);
            bool sound_found = (sound_it != sounds.end());
            if (sound_found) {
                auto *sound = sound_it->second;
                sound_effect->is_playing = (sound->getStatus() == render::AudioStatus::Playing);
            }
        }
    }

    for (size_t i = 0; i < std::min(musics.size(), triggers.size()); ++i) {
        auto &music = musics[i];
        auto &trigger = triggers[i];

        bool should_start = music && trigger && !trigger->triggered && !music->is_playing;
        if (!should_start) continue;

        // Get or create track (max 2 branches)
        auto track_it = music_tracks.find(music->music_path);
        render::IMusic *track = nullptr;

        if (track_it == music_tracks.end()) {
            track = audioManager.createMusic();
            music_tracks[music->music_path] = track;
        } else {
            track = track_it->second;
        }

        // Try to open and play
        bool opened = track->openFromFile(music->music_path);
        if (opened) {
            track->setVolume(music->volume);
            track->setLoop(music->loop);
            track->play();
            music->is_playing = true;
            trigger->triggered = true;
        }

        // Update playing status (max 2 branches but separated)
        bool should_check_music_status = music && music->is_playing;
        if (should_check_music_status) {
            auto track_it = music_tracks.find(music->music_path);
            bool track_found = (track_it != music_tracks.end());
            if (track_found) {
                auto *track = track_it->second;
                music->is_playing = (track->getStatus() == render::AudioStatus::Playing);
            }
        }
    }
}

} // namespace systems
