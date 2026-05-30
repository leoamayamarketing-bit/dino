#include "../../include/core/AudioManager.h"
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <iostream>

// ---------------------------------------------------------------------------
// PlaybackInstance destructor – ensures waveOut is closed and header unprepared
// ---------------------------------------------------------------------------
AudioManager::PlaybackInstance::~PlaybackInstance() {
    if (hWaveOut) {
        waveOutReset(hWaveOut);
        if (header.dwFlags & WHDR_PREPARED)
            waveOutUnprepareHeader(hWaveOut, &header, sizeof(header));
        waveOutClose(hWaveOut);
    }
}

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------
AudioManager::AudioManager() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

AudioManager::~AudioManager() {
    stopMusic();
    activeSounds_.clear(); // force destructors
}

// ---------------------------------------------------------------------------
// Generate all procedural sound buffers
// ---------------------------------------------------------------------------
void AudioManager::init() {
    // ---------- JUMP: quick rising sine sweep ----------
    {
        SoundParams p;
        p.duration = 0.18f;
        p.startFreq = 200.0f;
        p.endFreq = 700.0f;
        p.amplitude = 0.35f;
        p.attack = 0.005f;
        p.decay = 0.02f;
        p.sustainLevel = 0.8f;
        p.release = 0.05f;
        buffers_["jump"] = createSoundBuffer(p);
    }

    // ---------- COIN: bright two-tone chime ----------
    {
        float note1Dur = 0.06f;
        float note2Dur = 0.08f;
        float gap = 0.02f;
        float totalDur = note1Dur + gap + note2Dur + 0.05f;

        std::vector<int16_t> samples(static_cast<size_t>(totalDur * SAMPLE_RATE), 0);
        auto addNote = [&](float freq, float startTime, float dur, float amp) {
            size_t start = static_cast<size_t>(startTime * SAMPLE_RATE);
            size_t end   = static_cast<size_t>((startTime + dur) * SAMPLE_RATE);
            for (size_t i = start; i < end && i < samples.size(); ++i) {
                float t = static_cast<float>(i) / SAMPLE_RATE;
                float localT = t - startTime;
                float env = std::min(1.0f, localT / 0.005f);
                env *= std::max(0.0f, 1.0f - (localT - dur + 0.03f) / 0.03f);
                float sample = std::sin(2.0f * 3.14159f * freq * t);
                sample += std::sin(2.0f * 3.14159f * freq * 2.0f * t) * 0.3f;
                sample += std::sin(2.0f * 3.14159f * freq * 3.0f * t) * 0.15f;
                samples[i] += static_cast<int16_t>(sample * amp * env * 30000);
            }
        };
        addNote(880.0f, 0.0f, note1Dur, 0.4f);
        addNote(1319.0f, note1Dur + gap, note2Dur, 0.35f);

        SoundBuffer buf;
        buf.samples = std::move(samples);
        buf.sampleRate = SAMPLE_RATE;
        buffers_["coin"] = std::move(buf);
    }

    // ---------- DAMAGE: low thud with noise ----------
    {
        SoundParams p;
        p.duration = 0.25f;
        p.startFreq = 100.0f;
        p.endFreq = 40.0f;
        p.amplitude = 0.6f;
        p.noiseAmount = 0.5f;
        p.attack = 0.002f;
        p.decay = 0.08f;
        p.sustainLevel = 0.3f;
        p.release = 0.1f;
        buffers_["damage"] = createSoundBuffer(p);
    }

    // ---------- DASH: whoosh (rising sweep + noise) ----------
    {
        SoundParams p;
        p.duration = 0.3f;
        p.startFreq = 150.0f;
        p.endFreq = 900.0f;
        p.amplitude = 0.25f;
        p.noiseAmount = 0.4f;
        p.attack = 0.01f;
        p.decay = 0.05f;
        p.sustainLevel = 0.7f;
        p.release = 0.08f;
        buffers_["dash"] = createSoundBuffer(p);
    }

    // ---------- POWER-UP: ascending arpeggio ----------
    {
        float notes[] = {523.0f, 659.0f, 784.0f, 1047.0f}; // C5, E5, G5, C6
        float noteLen = 0.07f;
        float gap = 0.02f;
        float totalDur = (noteLen + gap) * 4 + 0.1f;

        std::vector<int16_t> samples(static_cast<size_t>(totalDur * SAMPLE_RATE), 0);
        auto addNote = [&](float freq, float startTime, float dur, float amp) {
            size_t start = static_cast<size_t>(startTime * SAMPLE_RATE);
            size_t end   = static_cast<size_t>((startTime + dur) * SAMPLE_RATE);
            for (size_t i = start; i < end && i < samples.size(); ++i) {
                float t = static_cast<float>(i) / SAMPLE_RATE;
                float localT = t - startTime;
                float env = std::min(1.0f, localT / 0.003f);
                env *= std::max(0.0f, 1.0f - (localT - dur + 0.03f) / 0.03f);
                float sample = std::sin(2.0f * 3.14159f * freq * t);
                sample += std::sin(2.0f * 3.14159f * freq * 2.0f * t) * 0.25f;
                samples[i] += static_cast<int16_t>(sample * amp * env * 28000);
            }
        };
        for (int i = 0; i < 4; ++i)
            addNote(notes[i], i * (noteLen + gap), noteLen, 0.35f - i * 0.03f);

        SoundBuffer buf;
        buf.samples = std::move(samples);
        buf.sampleRate = SAMPLE_RATE;
        buffers_["powerup"] = std::move(buf);
    }

    // ---------- GAME OVER: descending sad tone ----------
    {
        SoundParams p;
        p.duration = 0.8f;
        p.startFreq = 400.0f;
        p.endFreq = 80.0f;
        p.amplitude = 0.4f;
        p.noiseAmount = 0.1f;
        p.attack = 0.02f;
        p.decay = 0.1f;
        p.sustainLevel = 0.8f;
        p.release = 0.15f;
        buffers_["gameover"] = createSoundBuffer(p);
    }

    // ---------- MENU SELECT: short beep ----------
    {
        SoundParams p;
        p.duration = 0.06f;
        p.startFreq = 1000.0f;
        p.endFreq = 1000.0f;
        p.amplitude = 0.2f;
        p.attack = 0.001f;
        p.decay = 0.01f;
        p.sustainLevel = 1.0f;
        p.release = 0.02f;
        buffers_["select"] = createSoundBuffer(p);
    }

    // ---------- SHIELD HIT: shimmer ----------
    {
        SoundParams p;
        p.duration = 0.2f;
        p.startFreq = 2000.0f;
        p.endFreq = 4000.0f;
        p.amplitude = 0.2f;
        p.noiseAmount = 0.3f;
        p.attack = 0.002f;
        p.decay = 0.05f;
        p.sustainLevel = 0.5f;
        p.release = 0.08f;
        buffers_["shield"] = createSoundBuffer(p);
    }

    std::cout << "[Audio] Generated " << buffers_.size() << " procedural sound effects" << std::endl;
}

// ---------------------------------------------------------------------------
// Play a named sound effect
// ---------------------------------------------------------------------------
void AudioManager::playSound(const std::string& name) {
    if (muted_) return;

    auto it = buffers_.find(name);
    if (it == buffers_.end()) return;

    playRaw(it->second.samples, it->second.sampleRate);
}

// ---------------------------------------------------------------------------
// Background music
// ---------------------------------------------------------------------------
void AudioManager::startMusic() {
    stopMusic();

    float musicDuration = 16.0f; // 16-second loop
    float bpm = 140.0f;
    auto samples = generateMusicSamples(musicDuration, bpm);

    // Create a looping playback via a dedicated instance
    auto inst = std::make_unique<PlaybackInstance>();

    WAVEFORMATEX fmt;
    std::memset(&fmt, 0, sizeof(fmt));
    fmt.wFormatTag      = WAVE_FORMAT_PCM;
    fmt.nChannels       = 1;
    fmt.nSamplesPerSec  = SAMPLE_RATE;
    fmt.wBitsPerSample  = 16;
    fmt.nBlockAlign     = fmt.nChannels * fmt.wBitsPerSample / 8;
    fmt.nAvgBytesPerSec = fmt.nSamplesPerSec * fmt.nBlockAlign;

    if (waveOutOpen(&inst->hWaveOut, WAVE_MAPPER, &fmt,
                    reinterpret_cast<DWORD_PTR>(nullptr), 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        std::cerr << "[Audio] Failed to open music wave device" << std::endl;
        return;
    }

    // Set volume for music
    if (inst->hWaveOut) {
        DWORD vol = static_cast<DWORD>(musicVolume_ * 0xFFFF / 100);
        waveOutSetVolume(inst->hWaveOut, vol | (vol << 16));
    }

    inst->samples = std::move(samples);
    inst->isMusic = true;

    std::memset(&inst->header, 0, sizeof(inst->header));
    inst->header.lpData         = reinterpret_cast<LPSTR>(inst->samples.data());
    inst->header.dwBufferLength = static_cast<DWORD>(inst->samples.size() * sizeof(int16_t));
    inst->header.dwLoops        = 0; // we loop by re-queuing in update()

    if (waveOutPrepareHeader(inst->hWaveOut, &inst->header, sizeof(inst->header)) != MMSYSERR_NOERROR) {
        std::cerr << "[Audio] Failed to prepare music buffer" << std::endl;
        return;
    }

    if (waveOutWrite(inst->hWaveOut, &inst->header, sizeof(inst->header)) != MMSYSERR_NOERROR) {
        std::cerr << "[Audio] Failed to write music buffer" << std::endl;
        return;
    }

    musicInstance_ = std::move(inst);
}

void AudioManager::stopMusic() {
    musicInstance_.reset();
}

// ---------------------------------------------------------------------------
// Per-frame cleanup: remove finished sounds, re-queue looping music
// ---------------------------------------------------------------------------
void AudioManager::update() {
    // Clean up finished one-shot sounds
    activeSounds_.erase(
        std::remove_if(activeSounds_.begin(), activeSounds_.end(),
            [](const std::unique_ptr<PlaybackInstance>& inst) {
                if (!inst || !inst->hWaveOut) return true;
                // Check if playback has finished
                MMTIME mmt;
                std::memset(&mmt, 0, sizeof(mmt));
                mmt.wType = TIME_BYTES;
                // If waveOutGetPosition fails or returns 0 bytes remaining, it's done
                // Actually WAVEHDR.dwFlags & WHDR_DONE is the reliable check
                return (inst->header.dwFlags & WHDR_DONE) != 0;
            }),
        activeSounds_.end());

    // Re-queue music loop if it finished
    if (musicInstance_ && musicInstance_->hWaveOut) {
        if (musicInstance_->header.dwFlags & WHDR_DONE) {
            // Un-prepare and re-prepare to loop
            waveOutUnprepareHeader(musicInstance_->hWaveOut,
                                   &musicInstance_->header, sizeof(musicInstance_->header));
            musicInstance_->header.dwFlags = 0;
            waveOutPrepareHeader(musicInstance_->hWaveOut,
                                 &musicInstance_->header, sizeof(musicInstance_->header));
            waveOutWrite(musicInstance_->hWaveOut,
                         &musicInstance_->header, sizeof(musicInstance_->header));
        }
    }
}

// ---------------------------------------------------------------------------
// Volume control
// ---------------------------------------------------------------------------
void AudioManager::setSoundVolume(float volume) {
    soundVolume_ = std::clamp(volume, 0.0f, 100.0f);
}

void AudioManager::setMusicVolume(float volume) {
    musicVolume_ = std::clamp(volume, 0.0f, 100.0f);
    if (musicInstance_ && musicInstance_->hWaveOut) {
        DWORD vol = static_cast<DWORD>(musicVolume_ * 0xFFFF / 100);
        waveOutSetVolume(musicInstance_->hWaveOut, vol | (vol << 16));
    }
}

void AudioManager::setMuted(bool muted) {
    muted_ = muted;
    if (muted) {
        // Stop all active sounds
        if (musicInstance_ && musicInstance_->hWaveOut)
            waveOutPause(musicInstance_->hWaveOut);
    } else {
        if (musicInstance_ && musicInstance_->hWaveOut)
            waveOutRestart(musicInstance_->hWaveOut);
    }
}

// ---------------------------------------------------------------------------
// playRaw – spawn a waveOut device for one-shot playback
// ---------------------------------------------------------------------------
void AudioManager::playRaw(const std::vector<int16_t>& srcSamples, unsigned int sampleRate) {
    auto inst = std::make_unique<PlaybackInstance>();

    WAVEFORMATEX fmt;
    std::memset(&fmt, 0, sizeof(fmt));
    fmt.wFormatTag      = WAVE_FORMAT_PCM;
    fmt.nChannels       = 1;
    fmt.nSamplesPerSec  = sampleRate;
    fmt.wBitsPerSample  = 16;
    fmt.nBlockAlign     = fmt.nChannels * fmt.wBitsPerSample / 8;
    fmt.nAvgBytesPerSec = fmt.nSamplesPerSec * fmt.nBlockAlign;

    if (waveOutOpen(&inst->hWaveOut, WAVE_MAPPER, &fmt,
                    reinterpret_cast<DWORD_PTR>(nullptr), 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        return; // silently fail – audio is optional
    }

    // Set volume
    if (inst->hWaveOut) {
        DWORD vol = static_cast<DWORD>(soundVolume_ * 0xFFFF / 100);
        waveOutSetVolume(inst->hWaveOut, vol | (vol << 16));
    }

    // Copy samples into the instance so they outlive the playback
    inst->samples = srcSamples;
    inst->isMusic = false;

    std::memset(&inst->header, 0, sizeof(inst->header));
    inst->header.lpData         = reinterpret_cast<LPSTR>(inst->samples.data());
    inst->header.dwBufferLength = static_cast<DWORD>(inst->samples.size() * sizeof(int16_t));

    if (waveOutPrepareHeader(inst->hWaveOut, &inst->header, sizeof(inst->header)) != MMSYSERR_NOERROR)
        return;

    if (waveOutWrite(inst->hWaveOut, &inst->header, sizeof(inst->header)) != MMSYSERR_NOERROR)
        return;

    activeSounds_.push_back(std::move(inst));
}

// ---------------------------------------------------------------------------
// Sound generation helpers
// ---------------------------------------------------------------------------
AudioManager::SoundBuffer AudioManager::createSoundBuffer(const SoundParams& params) {
    SoundBuffer buf;
    buf.samples = generateSamples(params);
    buf.sampleRate = SAMPLE_RATE;
    return buf;
}

std::vector<int16_t> AudioManager::generateSamples(const SoundParams& params) {
    size_t numSamples = static_cast<size_t>(params.duration * SAMPLE_RATE);
    std::vector<int16_t> samples(numSamples, 0);

    for (size_t i = 0; i < numSamples; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float progress = t / params.duration;

        // Frequency sweep
        float freq = params.startFreq + (params.endFreq - params.startFreq) * progress;

        // Generate waveform
        float sample = 0.0f;
        if (params.useSquare) {
            sample = std::sin(2.0f * 3.14159f * freq * t) > 0 ? 1.0f : -1.0f;
        } else {
            sample = std::sin(2.0f * 3.14159f * freq * t);
            sample += std::sin(2.0f * 3.14159f * freq * 2.0f * t) * 0.3f;
            sample += std::sin(2.0f * 3.14159f * freq * 3.0f * t) * 0.1f;
            sample *= 0.7f;
        }

        // Add noise
        if (params.noiseAmount > 0.0f) {
            float noise = (static_cast<float>(std::rand()) / RAND_MAX * 2.0f - 1.0f);
            sample = sample * (1.0f - params.noiseAmount) + noise * params.noiseAmount;
        }

        // ADSR envelope
        float env = 0.0f;
        if (t < params.attack && params.attack > 0.0f)
            env = t / params.attack;
        else if (t < params.attack + params.decay) {
            float dt = t - params.attack;
            env = 1.0f - (1.0f - params.sustainLevel) * (dt / params.decay);
        } else if (t < params.duration - params.release)
            env = params.sustainLevel;
        else {
            float rt = t - (params.duration - params.release);
            env = params.sustainLevel * (1.0f - rt / params.release);
        }

        sample *= params.amplitude * env;
        sample = std::clamp(sample, -1.0f, 1.0f);
        samples[i] = static_cast<int16_t>(sample * 30000);
    }

    return samples;
}

std::vector<int16_t> AudioManager::generateMusicSamples(float duration, float bpm) {
    size_t numSamples = static_cast<size_t>(duration * SAMPLE_RATE);
    std::vector<int16_t> samples(numSamples, 0);

    // A minor pentatonic scale
    float notes[] = {
        440.0f, 523.0f, 587.0f, 659.0f, 784.0f,
        880.0f, 1047.0f, 1175.0f
    };
    int numNotes = sizeof(notes) / sizeof(notes[0]);

    // Bass notes (lower octave)
    float bassNotes[] = { 220.0f, 261.0f, 293.0f, 329.0f };
    int numBassNotes = sizeof(bassNotes) / sizeof(bassNotes[0]);

    float beatLen   = 60.0f / bpm;           // seconds per beat
    float measureLen = beatLen * 4;           // 4/4 time

    // Melody pattern (16th notes)
    int melodyPattern[] = {
        0, -1, 1, 2,  1, -1, 2, 3,
        4, -1, 3, 2,  1, -1, 0, -1,
        2, 3, 4, 3,  2, 1, 0, -1,
        1, 2, 3, 4,  3, 2, 1, 0
    };
    int patternLen = sizeof(melodyPattern) / sizeof(melodyPattern[0]);
    float noteLen  = measureLen / patternLen;

    // Bass pattern (quarter notes)
    int bassPattern[] = { 0, 1, 2, 1, 0, 2, 1, 2, 0, 2, 3, 1, 2, 3, 0, 1 };
    int bassPatternLen = sizeof(bassPattern) / sizeof(bassPattern[0]);
    float bassNoteLen  = measureLen / (bassPatternLen / 4.0f);

    // Chord pads (half notes)
    float chords[][3] = {
        {440.0f, 523.0f, 659.0f},
        {523.0f, 659.0f, 784.0f},
        {293.0f, 440.0f, 587.0f},
        {440.0f, 659.0f, 784.0f},
    };
    int numChords = sizeof(chords) / sizeof(chords[0]);

    for (size_t i = 0; i < numSamples; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float measurePos = std::fmod(t, measureLen);
        int measureIdx   = static_cast<int>(t / measureLen);
        float sample = 0.0f;

        // --- Melody (quiet, chirpy) ---
        {
            int noteIdx = melodyPattern[static_cast<int>(measurePos / noteLen) % patternLen];
            if (noteIdx >= 0 && noteIdx < numNotes) {
                float freq   = notes[noteIdx];
                float localT = std::fmod(measurePos, noteLen);
                float env    = std::max(0.0f, 1.0f - localT / noteLen * 2.0f);
                env = std::min(1.0f, localT / 0.01f) * env;
                float note = std::sin(2.0f * 3.14159f * freq * t);
                note += std::sin(2.0f * 3.14159f * freq * 2.0f * t) * 0.2f;
                sample += note * env * 0.08f;
            }
        }

        // --- Bass (pulsing) ---
        {
            int bassIdx = bassPattern[static_cast<int>(measurePos / bassNoteLen) % bassPatternLen];
            if (bassIdx >= 0 && bassIdx < numBassNotes) {
                float freq   = bassNotes[bassIdx];
                float localT = std::fmod(measurePos, bassNoteLen);
                float env    = std::max(0.0f, 1.0f - localT / (bassNoteLen * 0.5f));
                env = std::min(1.0f, localT / 0.005f) * env;
                float note   = std::sin(2.0f * 3.14159f * freq * t);
                note += std::sin(2.0f * 3.14159f * freq * 2.0f * t) * 0.3f;
                sample += note * env * 0.12f;
            }
        }

        // --- Chord pad (airy, quiet) ---
        {
            int chordIdx = measureIdx % numChords;
            float localT = std::fmod(measurePos, measureLen);
            float env = 0.5f + 0.5f * std::sin(3.14159f * localT / measureLen);
            for (int c = 0; c < 3; ++c) {
                float freq = chords[chordIdx][c];
                float note = std::sin(2.0f * 3.14159f * freq * t);
                note += std::sin(2.0f * 3.14159f * freq * 0.5f * t) * 0.5f;
                sample += note * env * 0.04f;
            }
        }

        // --- Percussion ---
        {
            float beatPos = std::fmod(measurePos, beatLen);
            if (beatPos < 0.03f) {
                float kick = std::sin(2.0f * 3.14159f * 60.0f * std::fmod(t, beatLen)) *
                             (1.0f - beatPos / 0.03f);
                sample += kick * 0.15f;
            }
            float eighthPos = std::fmod(measurePos, beatLen * 0.5f);
            if (eighthPos > beatLen * 0.35f && eighthPos < beatLen * 0.38f) {
                float noise = (static_cast<float>(std::rand()) / RAND_MAX * 2.0f - 1.0f);
                sample += noise * 0.03f;
            }
        }

        sample = std::clamp(sample, -1.0f, 1.0f);
        samples[i] = static_cast<int16_t>(sample * 25000);
    }

    return samples;
}
