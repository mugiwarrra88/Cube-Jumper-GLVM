// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "ProceduralMusicSystem.hpp"
#include <cmath>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <cstdlib>  // For rand()

namespace GLVM::core::Sound
{
    ProceduralMusicGenerator::ProceduralMusicGenerator()
        : rng(std::random_device{}())        , tempo_dist(0.8f, 1.5f)  // Faster tempo variations for energetic music
        , note_dist(0, 6)
        , volume_dist(0.6f, 0.9f) // Higher volume for more upbeat feel
        , currentScale(Scale::PENTATONIC)
        , baseTempo(220.0f) // Ultra high BPM for intense music
    {
        InitializeScale(currentScale);
    }

    void ProceduralMusicGenerator::InitializeScale(Scale scale)
    {
        scaleNotes.clear();
        currentScale = scale;
        
        switch (scale) {
            case Scale::MAJOR:
                scaleNotes = {Note::C4, Note::D4, Note::E4, Note::F4, Note::G4, Note::A4, Note::B4};
                break;
            case Scale::MINOR:
                scaleNotes = {Note::A4, Note::B4, Note::C5, Note::D5, Note::E5, Note::F5, Note::G5};
                break;
            case Scale::PENTATONIC:
                scaleNotes = {Note::C4, Note::D4, Note::E4, Note::G4, Note::A4, Note::C5, Note::D5};
                break;
            case Scale::BLUES:
                scaleNotes = {Note::C4, Note::E4, Note::F4, Note::G4, Note::A4, Note::C5};
                break;
        }
        
        note_dist = std::uniform_int_distribution<int>(0, scaleNotes.size() - 1);
    }

    void ProceduralMusicGenerator::SetScale(Scale scale)
    {
        InitializeScale(scale);
    }

    void ProceduralMusicGenerator::SetBaseTempo(float bpm)
    {
        baseTempo = bpm;
    }

    std::vector<int16_t> ProceduralMusicGenerator::GenerateTone(const ToneSettings& settings, int sampleRate)
    {
        float frequency = static_cast<float>(settings.note);
        int numSamples = static_cast<int>(settings.duration * sampleRate);
        std::vector<int16_t> audioData(numSamples);
        
        const float amplitude = 16384.0f * settings.volume; // Use 16-bit range
        const float pi2 = 2.0f * 3.14159265359f;
        
        for (int i = 0; i < numSamples; ++i) {
            float t = static_cast<float>(i) / sampleRate;
            float sample = 0.0f;
            
            switch (settings.waveform) {
                case 0: // Sine wave
                    sample = std::sin(pi2 * frequency * t);
                    break;
                case 1: // Square wave
                    sample = (std::sin(pi2 * frequency * t) > 0) ? 1.0f : -1.0f;
                    break;
                case 2: // Triangle wave
                    sample = (2.0f / 3.14159265359f) * std::asin(std::sin(pi2 * frequency * t));
                    break;
                case 3: // Sawtooth wave
                    sample = 2.0f * (t * frequency - std::floor(t * frequency + 0.5f));
                    break;
            }
            
            // Apply envelope (fade in/out to avoid clicks)
            float envelope = 1.0f;
            if (i < sampleRate * 0.01f) { // 10ms fade in
                envelope = static_cast<float>(i) / (sampleRate * 0.01f);
            } else if (i > numSamples - sampleRate * 0.01f) { // 10ms fade out
                envelope = static_cast<float>(numSamples - i) / (sampleRate * 0.01f);
            }
            
            audioData[i] = static_cast<int16_t>(sample * amplitude * envelope);
        }
        
        return audioData;
    }

    void ProceduralMusicGenerator::WriteWavFile(const std::string& filename, const std::vector<int16_t>& audioData, int sampleRate)
    {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << filename << std::endl;
            return;
        }
        
        // WAV header
        struct WavHeader {
            char riff[4] = {'R', 'I', 'F', 'F'};
            uint32_t fileSize;
            char wave[4] = {'W', 'A', 'V', 'E'};
            char fmt[4] = {'f', 'm', 't', ' '};
            uint32_t fmtSize = 16;
            uint16_t audioFormat = 1; // PCM
            uint16_t numChannels = 1; // Mono
            uint32_t sampleRate;
            uint32_t byteRate;
            uint16_t blockAlign = 2; // 16-bit mono
            uint16_t bitsPerSample = 16;
            char data[4] = {'d', 'a', 't', 'a'};
            uint32_t dataSize;
        } header;
        
        header.sampleRate = sampleRate;
        header.byteRate = sampleRate * 2; // 16-bit mono
        header.dataSize = audioData.size() * 2;
        header.fileSize = sizeof(header) - 8 + header.dataSize;
        
        file.write(reinterpret_cast<const char*>(&header), sizeof(header));
        file.write(reinterpret_cast<const char*>(audioData.data()), audioData.size() * 2);
        file.close();
    }

    std::string ProceduralMusicGenerator::GeneratePhrase(int noteCount)
    {
        std::vector<int16_t> fullPhrase;
        const int sampleRate = 22050;
          for (int i = 0; i < noteCount; ++i) {
            ToneSettings settings;
            settings.note = scaleNotes[note_dist(rng)];
            settings.duration = 60.0f / baseTempo * tempo_dist(rng) * 0.7f; // Shorter notes for faster feel
            settings.volume = volume_dist(rng);
            settings.waveform = (i % 2 == 0) ? 0 : 2; // Alternate between sine and triangle for brighter sound
            
            auto noteData = GenerateTone(settings, sampleRate);
            fullPhrase.insert(fullPhrase.end(), noteData.begin(), noteData.end());
            
            // Shorter pause between notes for tighter rhythm
            int pauseSamples = static_cast<int>(0.02f * sampleRate);
            fullPhrase.insert(fullPhrase.end(), pauseSamples, 0);
        }
        
        std::string filename = "procedural_phrase_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".wav";
        WriteWavFile(filename, fullPhrase, sampleRate);
        
        return filename;
    }    std::string ProceduralMusicGenerator::GenerateAmbient(float duration)
    {
        std::vector<int16_t> ambientData;
        const int sampleRate = 22050;
        const int totalSamples = static_cast<int>(duration * sampleRate);
        
        // Generate simple ambient tones
        for (int i = 0; i < totalSamples; ++i) {
            float t = static_cast<float>(i) / sampleRate;
            float sample = 0.0f;
            
            // Simple sine waves for ambient sound
            sample += 0.3f * std::sin(2.0f * 3.14159265359f * 220.0f * t); // A3
            sample += 0.2f * std::sin(2.0f * 3.14159265359f * 330.0f * t); // E4
            sample += 0.15f * std::sin(2.0f * 3.14159265359f * 440.0f * t); // A4
            
            // Simple volume envelope
            float envelope = 0.5f;
            if (t < 1.0f) {
                envelope = t * 0.5f; // fade in
            } else if (t > duration - 1.0f) {
                envelope = (duration - t) * 0.5f; // fade out
            }
            
            ambientData.push_back(static_cast<int16_t>(sample * 8192 * envelope));
        }
        
        std::string filename = "simple_ambient_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".wav";
        WriteWavFile(filename, ambientData, sampleRate);
        
        return filename;
    }

    std::string ProceduralMusicGenerator::GenerateRhythm(int measures)
    {
        std::vector<int16_t> rhythmData;
        const int sampleRate = 22050;
        const float beatDuration = 60.0f / baseTempo;
        const int beatsPerMeasure = 4;
        
        for (int measure = 0; measure < measures; ++measure) {
            for (int beat = 0; beat < beatsPerMeasure; ++beat) {
                // Emphasize downbeats
                float volume = (beat == 0) ? 0.8f : 0.4f;
                
                ToneSettings drumSettings;
                drumSettings.note = (beat == 0) ? Note::C4 : Note::G4; // Kick and snare pattern
                drumSettings.duration = beatDuration * 0.3f; // Short percussive sounds
                drumSettings.volume = volume;
                drumSettings.waveform = 1; // Square wave for percussive sound
                
                auto beatData = GenerateTone(drumSettings, sampleRate);
                rhythmData.insert(rhythmData.end(), beatData.begin(), beatData.end());
                
                // Fill rest of beat with silence
                int restSamples = static_cast<int>((beatDuration - drumSettings.duration) * sampleRate);
                rhythmData.insert(rhythmData.end(), restSamples, 0);
            }
        }
        
        std::string filename = "procedural_rhythm_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".wav";
        WriteWavFile(filename, rhythmData, sampleRate);        
        return filename;
    }

    // ProceduralMusicSystem implementation
      ProceduralMusicSystem::ProceduralMusicSystem(ISoundEngine* engine)
        : isRunning(false)
        , shouldStop(false)
        , soundEngine(engine)
        , nextPlayTime(0.0f)
        , currentTime(0.0f)    {
        generator.SetScale(Scale::MAJOR); // Major scale for happier sound
        generator.SetBaseTempo(220.0f); // Ultra high BPM for intense energetic music
    }

    ProceduralMusicSystem::~ProceduralMusicSystem()
    {
        Stop();
        CleanupOldFiles();
    }

    void ProceduralMusicSystem::Start()
    {
        if (!isRunning.load()) {
            shouldStop.store(false);
            isRunning.store(true);
            musicThread = std::thread(&ProceduralMusicSystem::MusicGenerationLoop, this);
        }
    }

    void ProceduralMusicSystem::Stop()
    {
        shouldStop.store(true);
        if (musicThread.joinable()) {
            musicThread.join();
        }
        isRunning.store(false);
    }

    void ProceduralMusicSystem::Update(float deltaTime)
    {
        currentTime += deltaTime;
    }

    void ProceduralMusicSystem::SetMusicStyle(Scale scale, float tempo)
    {
        generator.SetScale(scale);
        generator.SetBaseTempo(tempo);
    }

    void ProceduralMusicSystem::SetPlaybackInterval(float interval)
    {
        nextPlayTime = currentTime + interval;
    }    void ProceduralMusicSystem::MusicGenerationLoop()
    {
        while (!shouldStop.load()) {
            try {
                // Generate upbeat phrase with more notes for energetic feel
                std::string musicFile = generator.GeneratePhrase(12); // More notes for complexity
                
                // Play the generated music
                CSoundSample* pSoundSample = new CSoundSample();
                pSoundSample->kPath_to_File_ = musicFile.c_str();
                pSoundSample->uiDuration_ = 8; // Shorter tracks for variety
                pSoundSample->uiRate_ = 22050;
                
                if (soundEngine) {
                    soundEngine->GetSoundContainer().Push(pSoundSample);
                }
                
                generatedFiles.push_back(musicFile);
                
                // Shorter wait time for more frequent music changes
                std::this_thread::sleep_for(std::chrono::milliseconds(6000));
                
                // Clean up old files
                if (generatedFiles.size() > 5) { // Keep more files for overlapping
                    CleanupOldFiles();
                }
                
            } catch (const std::exception& e) {
                std::cerr << "Error in music generation: " << e.what() << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
    }

    void ProceduralMusicSystem::CleanupOldFiles()
    {
        // Remove old generated files to avoid disk space issues
        for (const auto& file : generatedFiles) {
            try {
                if (std::filesystem::exists(file)) {
                    std::filesystem::remove(file);
                }
            } catch (const std::exception& e) {
                std::cerr << "Error removing file " << file << ": " << e.what() << std::endl;
            }
        }
        generatedFiles.clear();
    }
}
