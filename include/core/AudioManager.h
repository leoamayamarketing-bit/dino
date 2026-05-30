#ifndef CORE_AUDIOMANAGER_H
#define CORE_AUDIOMANAGER_H

#include <windows.h>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <cstdint>

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    /// Generate all procedural sounds
    void init();

    /// Play a named sound effect
    void playSound(const std::string& name);

    /// Start/stop background music
    void startMusic();
    void stopMusic();

    /// Call every frame to clean up finished playback
    void update();

    /// Master volume control (0-100)
    void setSoundVolume(float volume);
    void setMusicVolume(float volume);

    /// Mute toggle
    void setMuted(bool muted);
    bool isMuted() const { return muted_; }

private:
    /// Generate PCM samples for a sound
    struct SoundParams {
        float duration = 0.2f;        // seconds
        float startFreq = 440.0f;     // Hz
        float endFreq = 440.0f;       // Hz (for sweeps)
        float amplitude = 0.5f;       // 0.0 - 1.0
        float noiseAmount = 0.0f;     // 0.0 - 1.0 (noise mixing)
        bool useSquare = false;       // square wave instead of sine
        float attack = 0.005f;        // seconds
        float decay = 0.03f;          // seconds
        float sustainLevel = 0.7f;    // 0.0 - 1.0
        float release = 0.05f;        // seconds
    };

    /// A buffer of raw PCM 16-bit mono samples
    struct SoundBuffer {
        std::vector<int16_t> samples;
        unsigned int sampleRate = 44100;
    };

    /// Active playback instance wrapping a Win32 waveOut handle
    struct PlaybackInstance {
        HWAVEOUT hWaveOut = nullptr;
        WAVEHDR header = {};
        std::vector<int16_t> samples;  // keeps PCM data alive
        bool isMusic = false;
        ~PlaybackInstance();  // ensures proper cleanup
    };

    /// Generate PCM samples from parameters
    std::vector<int16_t> generateSamples(const SoundParams& params);

    /// Generate background music samples (melody + bass + chords)
    std::vector<int16_t> generateMusicSamples(float duration, float bpm);

    /// Create a SoundBuffer from params
    SoundBuffer createSoundBuffer(const SoundParams& params);

    /// Actually play raw PCM samples via Win32 waveOut
    void playRaw(const std::vector<int16_t>& samples, unsigned int sampleRate);

    std::unordered_map<std::string, SoundBuffer> buffers_;
    std::vector<std::unique_ptr<PlaybackInstance>> activeSounds_;
    std::unique_ptr<PlaybackInstance> musicInstance_;

    float soundVolume_ = 80.0f;
    float musicVolume_ = 50.0f;
    bool muted_ = false;

    static constexpr unsigned int SAMPLE_RATE = 44100;
};

#endif
