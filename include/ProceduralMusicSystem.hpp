// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef PROCEDURAL_MUSIC_SYSTEM_HPP
#define PROCEDURAL_MUSIC_SYSTEM_HPP

#include "ISoundEngine.hpp"
#include "Vector.hpp"
#include <random>
#include <chrono>
#include <thread>
#include <atomic>
#include <vector>
#include <memory>

namespace GLVM::core::Sound
{
    // Musical notes frequencies (in Hz)
    enum class Note {
        C4 = 262, D4 = 294, E4 = 330, F4 = 349, G4 = 392, A4 = 440, B4 = 494,
        C5 = 523, D5 = 587, E5 = 659, F5 = 698, G5 = 784, A5 = 880, B5 = 988
    };

    // Musical scales
    enum class Scale {
        MAJOR,      // C-D-E-F-G-A-B
        MINOR,      // A-B-C-D-E-F-G (natural minor)
        PENTATONIC, // C-D-E-G-A
        BLUES       // C-Eb-F-Gb-G-Bb
    };

    struct ToneSettings {
        Note note;
        float duration;     // in seconds
        float volume;       // 0.0 to 1.0
        int waveform;      // 0=sine, 1=square, 2=triangle, 3=sawtooth
    };

    class ProceduralMusicGenerator {
    private:
        std::mt19937 rng;
        std::uniform_real_distribution<float> tempo_dist;
        std::uniform_int_distribution<int> note_dist;
        std::uniform_real_distribution<float> volume_dist;
        
        Scale currentScale;
        std::vector<Note> scaleNotes;
        float baseTempo;
        
        void InitializeScale(Scale scale);
        std::vector<int16_t> GenerateTone(const ToneSettings& settings, int sampleRate);
        void WriteWavFile(const std::string& filename, const std::vector<int16_t>& audioData, int sampleRate);
        
    public:
        ProceduralMusicGenerator();
        ~ProceduralMusicGenerator() = default;
        
        void SetScale(Scale scale);
        void SetBaseTempo(float bpm);
        
        // Generate a single musical phrase and save as WAV
        std::string GeneratePhrase(int noteCount = 8);
        
        // Generate ambient background tones
        std::string GenerateAmbient(float duration = 10.0f);
        
        // Generate rhythmic patterns
        std::string GenerateRhythm(int measures = 4);
    };

    class ProceduralMusicSystem {
    private:
        std::atomic<bool> isRunning;
        std::atomic<bool> shouldStop;
        std::thread musicThread;
        
        ISoundEngine* soundEngine;
        ProceduralMusicGenerator generator;
        
        float nextPlayTime;
        float currentTime;
        std::vector<std::string> generatedFiles;
        
        void MusicGenerationLoop();
        void CleanupOldFiles();
        
    public:
        ProceduralMusicSystem(ISoundEngine* engine);
        ~ProceduralMusicSystem();
        
        void Start();
        void Stop();
        void Update(float deltaTime);
        
        // Configuration
        void SetMusicStyle(Scale scale, float tempo);
        void SetPlaybackInterval(float interval); // Time between new musical phrases
        
        bool IsRunning() const { return isRunning.load(); }
    };
}

#endif // PROCEDURAL_MUSIC_SYSTEM_HPP
