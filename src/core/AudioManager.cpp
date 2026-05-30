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

    // ---------- LAND: soft thud ----------
    {
        SoundParams p;
        p.duration = 0.1f;
        p.startFreq = 60.0f;
        p.endFreq = 30.0f;
        p.amplitude = 0.25f;
        p.noiseAmount = 0.3f;
        p.attack = 0.001f;
        p.decay = 0.03f;
        p.sustainLevel = 0.4f;
        p.release = 0.04f;
        buffers_["land"] = createSoundBuffer(p);
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
void AudioManager::startMusic(Constants::LevelType theme) {
    stopMusic();

    currentTheme_ = theme;

    float musicDuration = 16.0f; // 16-second loop
    std::vector<int16_t> samples;

    switch (theme) {
        case Constants::LevelType::DESERT_NIGHT:
            samples = generateMusicDesertNight(musicDuration);
            break;
        case Constants::LevelType::CAVE:
            samples = generateMusicCave(musicDuration);
            break;
        case Constants::LevelType::VOLCANO:
            samples = generateMusicVolcano(musicDuration);
            break;
        default:
            samples = generateMusicDesertDay(musicDuration);
            break;
    }

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
    std::cout << "[Audio] Started music theme: " << static_cast<int>(theme) << std::endl;
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

// ===========================================================================
// Theme: DESERT DAY — bright, energetic, adventurous (150 BPM, C major)
// ===========================================================================
std::vector<int16_t> AudioManager::generateMusicDesertDay(float duration) {
    size_t numSamples = static_cast<size_t>(duration * SAMPLE_RATE);
    std::vector<int16_t> samples(numSamples, 0);

    // C major pentatonic scale (bright & happy)
    float notes[] = {
        523.0f, 587.0f, 659.0f, 784.0f, 880.0f,
        1047.0f, 1175.0f, 1319.0f, 1568.0f
    };
    int numNotes = sizeof(notes) / sizeof(notes[0]);

    // Bass notes (lower octave) — C major triad roots
    float bassNotes[] = { 262.0f, 294.0f, 330.0f, 349.0f, 392.0f, 440.0f };
    int numBassNotes = sizeof(bassNotes) / sizeof(bassNotes[0]);

    float bpm        = 150.0f;
    float beatLen    = 60.0f / bpm;
    float measureLen = beatLen * 4;

    // Cheerful melody pattern (16th notes, arpeggiated)
    int melodyPattern[] = {
        0, 2, 4, 6,   4, 2, 0, 2,
        1, 3, 5, 7,   5, 3, 1, -1,
        0, 1, 2, 3,   4, 5, 6, 7,
        6, 5, 4, 3,   2, 1, 0, -1,
        2, 4, 6, 8,   6, 4, 2, 0,
        1, 3, 5, 7,   5, 3, 1, -1,
        0, -1, 4, -1,  7, -1, 4, -1,
        6, 5, 4, 2,   0, 2, 4, 6
    };
    int patternLen = sizeof(melodyPattern) / sizeof(melodyPattern[0]);
    float noteLen  = measureLen / (patternLen / 2);

    // Walking bass line (eighth notes)
    int bassPattern[] = { 0, 1, 2, 3, 0, 2, 3, 4, 0, 2, 4, 5, 3, 1, 0, 1 };
    int bassPatternLen = sizeof(bassPattern) / sizeof(bassPattern[0]);
    float bassNoteLen  = measureLen / (bassPatternLen / 2);

    // Chord progression (C - F - C - G - C - Am - F - G)
    float chords[][3] = {
        {523.0f, 659.0f, 784.0f},  // C major
        {523.0f, 698.0f, 880.0f},  // F major
        {523.0f, 659.0f, 784.0f},  // C major
        {392.0f, 523.0f, 659.0f},  // G major
        {523.0f, 659.0f, 784.0f},  // C major
        {440.0f, 523.0f, 659.0f},  // A minor
        {523.0f, 698.0f, 880.0f},  // F major
        {392.0f, 523.0f, 659.0f},  // G major
    };
    int numChords = sizeof(chords) / sizeof(chords[0]);

    for (size_t i = 0; i < numSamples; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float measurePos = std::fmod(t, measureLen);
        int measureIdx   = static_cast<int>(t / measureLen);
        float sample = 0.0f;

        // --- Melody (bright, chirpy, high register) ---
        {
            int noteIdx = melodyPattern[static_cast<int>(measurePos / noteLen) % patternLen];
            if (noteIdx >= 0 && noteIdx < numNotes) {
                float freq   = notes[noteIdx];
                float localT = std::fmod(measurePos, noteLen);
                float env    = std::max(0.0f, 1.0f - localT / noteLen * 2.0f);
                env = std::min(1.0f, localT / 0.008f) * env;
                float note = std::sin(2.0f * 3.14159f * freq * t);
                note += std::sin(2.0f * 3.14159f * freq * 2.0f * t) * 0.25f;
                note += std::sin(2.0f * 3.14159f * freq * 3.0f * t) * 0.1f;
                sample += note * env * 0.07f;
            }
        }

        // --- Walking bass ---
        {
            int bassIdx = bassPattern[static_cast<int>(measurePos / bassNoteLen) % bassPatternLen];
            if (bassIdx >= 0 && bassIdx < numBassNotes) {
                float freq   = bassNotes[bassIdx];
                float localT = std::fmod(measurePos, bassNoteLen);
                float env    = std::max(0.0f, 1.0f - localT / (bassNoteLen * 0.4f));
                env = std::min(1.0f, localT / 0.005f) * env;
                float note   = std::sin(2.0f * 3.14159f * freq * t);
                note += std::sin(2.0f * 3.14159f * freq * 2.0f * t) * 0.3f;
                sample += note * env * 0.15f;
            }
        }

        // --- Chords (strummed, bright) ---
        {
            int chordIdx = measureIdx % numChords;
            float stride = std::fmod(measurePos, measureLen) / 8.0f;
            float env = 0.5f + 0.5f * std::sin(3.14159f * std::fmod(measurePos, measureLen) / measureLen);
            for (int c = 0; c < 3; ++c) {
                float freq = chords[chordIdx][c];
                float note = std::sin(2.0f * 3.14159f * freq * t + stride);
                note += std::sin(2.0f * 3.14159f * freq * 0.5f * t) * 0.3f;
                sample += note * env * 0.03f;
            }
        }

        // --- Percussion (kick + snare + hi-hat) ---
        {
            float beatPos = std::fmod(measurePos, beatLen);
            // Kick on every downbeat
            if (beatPos < 0.035f) {
                float kick = std::sin(2.0f * 3.14159f * 50.0f * std::fmod(t, beatLen)) *
                             (1.0f - beatPos / 0.035f);
                sample += kick * 0.18f;
            }
            // Snare on 2 and 4 (every other beat)
            int beat = static_cast<int>(measurePos / beatLen);
            if ((beat % 4 == 1 || beat % 4 == 3) && beatPos > beatLen * 0.45f && beatPos < beatLen * 0.50f) {
                float noise = (static_cast<float>(std::rand()) / RAND_MAX * 2.0f - 1.0f);
                float snare = noise * (1.0f - (beatPos - beatLen * 0.45f) / 0.05f);
                sample += snare * 0.10f;
            }
            // Hi-hat on off-beats (eighth notes)
            float eighthPos = std::fmod(measurePos, beatLen * 0.5f);
            if (eighthPos > beatLen * 0.35f && eighthPos < beatLen * 0.40f) {
                float hat = std::sin(2.0f * 3.14159f * 8000.0f * std::fmod(t, beatLen * 0.5f)) *
                            (1.0f - (eighthPos - beatLen * 0.35f) / 0.05f);
                hat = hat * 0.5f + 0.5f * (static_cast<float>(std::rand()) / RAND_MAX * 2.0f - 1.0f);
                sample += hat * 0.04f;
            }
        }

        sample = std::clamp(sample, -1.0f, 1.0f);
        samples[i] = static_cast<int16_t>(sample * 25000);
    }

    return samples;
}

// ===========================================================================
// Theme: DESERT NIGHT — mysterious, calm, atmospheric (95 BPM, A minor)
// ===========================================================================
std::vector<int16_t> AudioManager::generateMusicDesertNight(float duration) {
    size_t numSamples = static_cast<size_t>(duration * SAMPLE_RATE);
    std::vector<int16_t> samples(numSamples, 0);

    // A natural minor (melancholic, mysterious)
    float notes[] = {
        440.0f, 493.0f, 523.0f, 587.0f, 659.0f,
        698.0f, 784.0f, 880.0f, 987.0f
    };
    int numNotes = sizeof(notes) / sizeof(notes[0]);

    float bassNotes[] = { 110.0f, 131.0f, 147.0f, 165.0f, 196.0f, 220.0f };
    int numBassNotes = sizeof(bassNotes) / sizeof(bassNotes[0]);

    float bpm        = 95.0f;
    float beatLen    = 60.0f / bpm;
    float measureLen = beatLen * 4;

    // Winding, mysterious melody (eighth-note feel, lots of space)
    int melodyPattern[] = {
        0, -1, 2, -1,  1, -1, 3, -1,
        2, -1, 4, -1,  3, -1, 0, -1,
        1, -1, 3, -1,  4, -1, 5, -1,
        3, -1, 2, -1,  1, -1, 0, -1,
        0, 2, 1, 3,  2, 4, 3, 5,
        4, -1, 3, -1,  2, -1, 1, -1,
        2, 1, 3, 2,  4, 3, 5, 4,
        3, -1, 2, -1,  0, 2, 1, -1
    };
    int patternLen = sizeof(melodyPattern) / sizeof(melodyPattern[0]);
    float noteLen  = measureLen / (patternLen / 2);

    // Deep, slow bass pulse (half-notes)
    int bassPattern[] = { 0, 1, 2, 1, 0, -1, 2, 3 };
    int bassPatternLen = sizeof(bassPattern) / sizeof(bassPattern[0]);
    float bassNoteLen  = measureLen / (bassPatternLen / 2);

    // Minor chord pads (Am - Dm - Em - Am - Dm - Em - F - E7)
    float chords[][3] = {
        {440.0f, 523.0f, 659.0f},  // Am
        {587.0f, 698.0f, 880.0f},  // Dm
        {329.0f, 440.0f, 523.0f},  // Em
        {440.0f, 523.0f, 659.0f},  // Am
        {587.0f, 698.0f, 880.0f},  // Dm
        {329.0f, 440.0f, 523.0f},  // Em
        {523.0f, 659.0f, 784.0f},  // F
        {659.0f, 784.0f, 1047.0f}, // E7 (no 3rd, just 5th and flat 7th)
    };
    int numChords = sizeof(chords) / sizeof(chords[0]);

    for (size_t i = 0; i < numSamples; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float measurePos = std::fmod(t, measureLen);
        int measureIdx   = static_cast<int>(t / measureLen);
        float sample = 0.0f;

        // --- Melody (gentle, airy, lots of space) ---
        {
            int noteIdx = melodyPattern[static_cast<int>(measurePos / noteLen) % patternLen];
            if (noteIdx >= 0 && noteIdx < numNotes) {
                float freq   = notes[noteIdx];
                float localT = std::fmod(measurePos, noteLen);
                float env = std::max(0.0f, 1.0f - localT / noteLen);
                env = std::min(1.0f, localT / 0.02f) * env;
                float note = std::sin(2.0f * 3.14159f * freq * t);
                note += std::sin(2.0f * 3.14159f * freq * 0.5f * t) * 0.4f; // sub octave for warmth
                sample += note * env * 0.06f;
            }
        }

        // --- Bass (deep, sustained, pulsing) ---
        {
            int bassIdx = bassPattern[static_cast<int>(measurePos / bassNoteLen) % bassPatternLen];
            if (bassIdx >= 0 && bassIdx < numBassNotes) {
                float freq   = bassNotes[bassIdx];
                float localT = std::fmod(measurePos, bassNoteLen);
                float env    = std::max(0.0f, 1.0f - localT / (bassNoteLen * 0.7f));
                env = std::min(1.0f, localT / 0.01f) * env;
                float note   = std::sin(2.0f * 3.14159f * freq * t);
                note += std::sin(2.0f * 3.14159f * freq * 2.0f * t) * 0.2f;
                sample += note * env * 0.18f;
            }
        }

        // --- Chord pads (soft, airy, string-like) ---
        {
            int chordIdx = measureIdx % numChords;
            float env = 0.6f + 0.4f * std::cos(3.14159f * std::fmod(measurePos, measureLen) / measureLen);
            for (int c = 0; c < 3; ++c) {
                float freq = chords[chordIdx][c];
                float note = std::sin(2.0f * 3.14159f * freq * t);
                note += std::sin(2.0f * 3.14159f * freq * 1.01f * t) * 0.3f; // slight detune for warmth
                sample += note * env * 0.04f;
            }
        }

        // --- Percussion (gentle shaker, sparse kick) ---
        {
            float beatPos = std::fmod(measurePos, beatLen);
            // Soft kick on 1 and 3
            int beat = static_cast<int>(measurePos / beatLen);
            if ((beat % 4 == 0) && beatPos < 0.04f) {
                float kick = std::sin(2.0f * 3.14159f * 40.0f * std::fmod(t, beatLen)) *
                             (1.0f - beatPos / 0.04f);
                sample += kick * 0.12f;
            }
            // Shaker on off-beats
            float eighthPos = std::fmod(measurePos, beatLen * 0.5f);
            if (eighthPos > beatLen * 0.25f && eighthPos < beatLen * 0.30f) {
                float noise = (static_cast<float>(std::rand()) / RAND_MAX * 2.0f - 1.0f);
                float shaker = noise * (1.0f - (eighthPos - beatLen * 0.25f) / 0.05f);
                sample += shaker * 0.02f;
            }
        }

        sample = std::clamp(sample, -1.0f, 1.0f);
        samples[i] = static_cast<int16_t>(sample * 22000);
    }

    return samples;
}

// ===========================================================================
// Theme: CAVE — dark, echoey, deep (80 BPM, D phrygian)
// ===========================================================================
std::vector<int16_t> AudioManager::generateMusicCave(float duration) {
    size_t numSamples = static_cast<size_t>(duration * SAMPLE_RATE);
    std::vector<int16_t> samples(numSamples, 0);

    // D phrygian: D-D#-F-G-A-A#-C-D (dark, exotic)
    float notes[] = {
        293.0f, 311.0f, 349.0f, 391.0f, 440.0f, 466.0f, 523.0f, 587.0f
    };
    int numNotes = sizeof(notes) / sizeof(notes[0]);

    // Deep sub-bass notes
    float bassNotes[] = { 73.0f, 78.0f, 98.0f, 110.0f, 117.0f, 131.0f };
    int numBassNotes = sizeof(bassNotes) / sizeof(bassNotes[0]);

    float bpm        = 80.0f;
    float beatLen    = 60.0f / bpm;
    float measureLen = beatLen * 4;

    // Sparse, mysterious melody — lots of rests (-1)
    int melodyPattern[] = {
        0, -1, -1, 2,  -1, -1, 3, -1,
        -1, 2, -1, -1,  0, -1, -1, -1,
        1, -1, -1, 3,  -1, -1, 4, -1,
        -1, 3, -1, -1,  1, -1, -1, -1,
        2, -1, -1, 4,  -1, -1, 5, -1,
        -1, 4, -1, -1,  2, -1, -1, -1,
        0, -1, 2, -1,  4, -1, 5, -1,
        4, -1, 3, -1,  2, -1, 1, 0
    };
    int patternLen = sizeof(melodyPattern) / sizeof(melodyPattern[0]);
    float noteLen  = measureLen / (patternLen / 2);

    // Very slow bass (half-notes)
    int bassPattern[] = { 0, -1, 1, 2, 0, 2, 3, 1 };
    int bassPatternLen = sizeof(bassPattern) / sizeof(bassPattern[0]);
    float bassNoteLen  = measureLen / (bassPatternLen / 2);

    // Dark, suspended chords (Dm - D#dim - Gm - A# - Dm - Gm - A# - D#dim)
    float chords[][3] = {
        {293.0f, 349.0f, 440.0f},  // Dm
        {311.0f, 349.0f, 466.0f},  // D#dim
        {196.0f, 293.0f, 391.0f},  // Gm
        {233.0f, 349.0f, 466.0f},  // A#
        {293.0f, 349.0f, 440.0f},  // Dm
        {196.0f, 293.0f, 391.0f},  // Gm
        {233.0f, 349.0f, 466.0f},  // A#
        {311.0f, 349.0f, 466.0f},  // D#dim
    };
    int numChords = sizeof(chords) / sizeof(chords[0]);

    for (size_t i = 0; i < numSamples; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float measurePos = std::fmod(t, measureLen);
        int measureIdx   = static_cast<int>(t / measureLen);
        float sample = 0.0f;

        // --- Melody (low, dark, echoey) ---
        {
            int noteIdx = melodyPattern[static_cast<int>(measurePos / noteLen) % patternLen];
            if (noteIdx >= 0 && noteIdx < numNotes) {
                float freq   = notes[noteIdx];
                float localT = std::fmod(measurePos, noteLen);
                float env = std::max(0.0f, 1.0f - localT / noteLen);
                env = std::min(1.0f, localT / 0.03f) * env;
                float note = std::sin(2.0f * 3.14159f * freq * t);
                note += std::sin(2.0f * 3.14159f * freq * 0.5f * t) * 0.5f; // sub octave
                note += std::sin(2.0f * 3.14159f * freq * 1.005f * t) * 0.3f; // slight detune
                sample += note * env * 0.07f;
            }
        }

        // --- Bass (very deep, rumbling sub, sustained) ---
        {
            int bassIdx = bassPattern[static_cast<int>(measurePos / bassNoteLen) % bassPatternLen];
            if (bassIdx >= 0 && bassIdx < numBassNotes) {
                float freq   = bassNotes[bassIdx];
                float localT = std::fmod(measurePos, bassNoteLen);
                float env    = std::max(0.0f, 1.0f - localT / (bassNoteLen * 0.9f));
                env = std::min(1.0f, localT / 0.02f) * env;
                float note   = std::sin(2.0f * 3.14159f * freq * t);
                note += std::sin(2.0f * 3.14159f * freq * 3.0f * t) * 0.2f; // harmonic
                sample += note * env * 0.22f;
            }
        }

        // --- Chord pads (dark, hushed, subtle) ---
        {
            int chordIdx = measureIdx % numChords;
            float env = 0.4f + 0.6f * std::cos(3.14159f * std::fmod(measurePos, measureLen * 0.5f) / (measureLen * 0.5f));
            for (int c = 0; c < 3; ++c) {
                float freq = chords[chordIdx][c];
                float note = std::sin(2.0f * 3.14159f * freq * t);
                note += std::sin(2.0f * 3.14159f * freq * 1.02f * t) * 0.2f;
                sample += note * env * 0.03f;
            }
        }

        // --- Percussion (low drums, like dripping water echoes) ---
        {
            float beatPos = std::fmod(measurePos, beatLen);
            // Low tom/drip on 1 and 3
            int beat = static_cast<int>(measurePos / beatLen);
            if ((beat % 4 == 0 || beat % 4 == 2) && beatPos < 0.06f) {
                float drum = std::sin(2.0f * 3.14159f * 80.0f * std::fmod(t, beatLen)) *
                             (1.0f - beatPos / 0.06f);
                drum += std::sin(2.0f * 3.14159f * 120.0f * std::fmod(t, beatLen)) * 0.3f *
                        (1.0f - beatPos / 0.04f);
                sample += drum * 0.12f;
            }
            // Echoey drip on beat 2 and 4 (high-pitched, short)
            if ((beat % 4 == 1 || beat % 4 == 3) && beatPos > beatLen * 0.2f && beatPos < beatLen * 0.23f) {
                float drip = std::sin(2.0f * 3.14159f * 2000.0f * std::fmod(t, beatLen * 0.5f)) *
                             (1.0f - (beatPos - beatLen * 0.2f) / 0.03f);
                sample += drip * 0.04f;
            }
        }

        sample = std::clamp(sample, -1.0f, 1.0f);
        samples[i] = static_cast<int16_t>(sample * 20000);
    }

    return samples;
}

// ===========================================================================
// Theme: VOLCANO — intense, aggressive, driving (180 BPM, F harmonic minor)
// ===========================================================================
std::vector<int16_t> AudioManager::generateMusicVolcano(float duration) {
    size_t numSamples = static_cast<size_t>(duration * SAMPLE_RATE);
    std::vector<int16_t> samples(numSamples, 0);

    // F harmonic minor: F-G-G#-A#-C-D#-E-F (tense, aggressive)
    float notes[] = {
        349.0f, 392.0f, 415.0f, 466.0f, 523.0f, 622.0f, 659.0f, 698.0f,
        784.0f, 831.0f, 932.0f, 1047.0f
    };
    int numNotes = sizeof(notes) / sizeof(notes[0]);

    // Bass power-chord roots
    float bassNotes[] = { 87.0f, 98.0f, 104.0f, 117.0f, 131.0f, 139.0f, 175.0f };
    int numBassNotes = sizeof(bassNotes) / sizeof(bassNotes[0]);

    float bpm        = 180.0f;
    float beatLen    = 60.0f / bpm;
    float measureLen = beatLen * 4;

    // Aggressive, fast melody (16th notes, stepwise with leaps)
    int melodyPattern[] = {
        0, 2, 3, 5,   6, 5, 3, 2,   0, 2, 3, 5,   6, 7, 8, 7,
        5, 3, 5, 6,   7, 6, 5, 3,   2, 0, 2, 3,   5, 3, 2, 0,
        1, 3, 4, 5,   6, 5, 4, 3,   1, 3, 4, 5,   6, 8, 6, 5,
        3, 2, 3, 5,   6, 7, 6, 5,   3, 2, 0, 2,   3, 5, 7, 8
    };
    int patternLen = sizeof(melodyPattern) / sizeof(melodyPattern[0]);
    float noteLen  = measureLen / (patternLen / 2);

    // Driving bass (eighth notes)
    int bassPattern[] = { 0, 0, 1, 1, 2, 2, 1, 1, 0, 0, 2, 2, 3, 3, 1, 1 };
    int bassPatternLen = sizeof(bassPattern) / sizeof(bassPattern[0]);
    float bassNoteLen  = measureLen / (bassPatternLen / 2);

    // Tense chord progression (Fm - Cm - D# - G# - Fm - D# - Cm - G#)
    float chords[][3] = {
        {349.0f, 415.0f, 523.0f},  // Fm
        {523.0f, 622.0f, 784.0f},  // Cm (harmonic equivalent)
        {311.0f, 349.0f, 466.0f},  // D#
        {415.0f, 523.0f, 622.0f},  // G#
        {349.0f, 415.0f, 523.0f},  // Fm
        {311.0f, 349.0f, 466.0f},  // D#
        {523.0f, 622.0f, 784.0f},  // Cm
        {415.0f, 523.0f, 622.0f},  // G#
    };
    int numChords = sizeof(chords) / sizeof(chords[0]);

    for (size_t i = 0; i < numSamples; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float measurePos = std::fmod(t, measureLen);
        int measureIdx   = static_cast<int>(t / measureLen);
        float sample = 0.0f;

        // --- Melody (square wave, aggressive, in-your-face) ---
        {
            int noteIdx = melodyPattern[static_cast<int>(measurePos / noteLen) % patternLen];
            if (noteIdx >= 0 && noteIdx < numNotes) {
                float freq   = notes[noteIdx];
                float localT = std::fmod(measurePos, noteLen);
                float env    = std::max(0.0f, 1.0f - localT / noteLen * 2.0f);
                env = std::min(1.0f, localT / 0.005f) * env;
                // Square wave for aggressive tone
                float note = 0.0f;
                for (int h = 1; h <= 7; h += 2) { // square = odd harmonics
                    note += std::sin(2.0f * 3.14159f * freq * h * t) / h;
                }
                note *= 0.6f;
                note += std::sin(2.0f * 3.14159f * freq * 2.0f * t) * 0.2f;
                sample += note * env * 0.09f;
            }
        }

        // --- Bass (driving, pumping eighth notes) ---
        {
            int bassIdx = bassPattern[static_cast<int>(measurePos / bassNoteLen) % bassPatternLen];
            if (bassIdx >= 0 && bassIdx < numBassNotes) {
                float freq   = bassNotes[bassIdx];
                float localT = std::fmod(measurePos, bassNoteLen);
                float env    = std::max(0.0f, 1.0f - localT / (bassNoteLen * 0.6f));
                env = std::min(1.0f, localT / 0.005f) * env;
                float note   = std::sin(2.0f * 3.14159f * freq * t);
                note += std::sin(2.0f * 3.14159f * freq * 3.0f * t) * 0.3f; // distortion
                note = std::clamp(note, -1.0f, 1.0f); // clip for distortion
                sample += note * env * 0.18f;
            }
        }

        // --- Chord stabs (tense, short hits) ---
        {
            int chordIdx = measureIdx % numChords;
            // Stab on beat 1 and 3, quick decay
            float stabEnv = 0.0f;
            int beat = static_cast<int>(measurePos / beatLen);
            if (beat % 2 == 0) {
                float bp = std::fmod(measurePos, beatLen);
                if (bp < beatLen * 0.25f) {
                    stabEnv = 1.0f - bp / (beatLen * 0.25f);
                }
            }
            if (stabEnv > 0.0f) {
                for (int c = 0; c < 3; ++c) {
                    float freq = chords[chordIdx][c];
                    float note = std::sin(2.0f * 3.14159f * freq * t);
                    note += std::sin(2.0f * 3.14159f * freq * 2.0f * t) * 0.3f;
                    sample += note * stabEnv * 0.03f;
                }
            }
        }

        // --- Percussion (heavy kick + snare, driving) ---
        {
            float beatPos = std::fmod(measurePos, beatLen);
            int beat = static_cast<int>(measurePos / beatLen);
            // Heavy kick on every beat
            if (beatPos < 0.04f) {
                float kick = std::sin(2.0f * 3.14159f * 45.0f * std::fmod(t, beatLen)) *
                             (1.0f - beatPos / 0.04f);
                kick += std::sin(2.0f * 3.14159f * 90.0f * std::fmod(t, beatLen)) * 0.5f *
                        (1.0f - beatPos / 0.02f);
                sample += kick * 0.22f;
            }
            // Snare on 2 and 4 (heavy)
            if ((beat % 4 == 1 || beat % 4 == 3) && beatPos > beatLen * 0.48f && beatPos < beatLen * 0.54f) {
                float noise = (static_cast<float>(std::rand()) / RAND_MAX * 2.0f - 1.0f);
                float snare = noise * (1.0f - (beatPos - beatLen * 0.48f) / 0.06f);
                snare += std::sin(2.0f * 3.14159f * 200.0f * std::fmod(t, beatLen)) * 0.3f *
                         (1.0f - (beatPos - beatLen * 0.48f) / 0.04f);
                sample += snare * 0.12f;
            }
            // Hi-hat on eighth-note offbeats
            float eighthPos = std::fmod(measurePos, beatLen * 0.5f);
            if (eighthPos > beatLen * 0.38f && eighthPos < beatLen * 0.42f) {
                float noise = (static_cast<float>(std::rand()) / RAND_MAX * 2.0f - 1.0f);
                float hat = noise * (1.0f - (eighthPos - beatLen * 0.38f) / 0.04f);
                sample += hat * 0.05f;
            }
        }

        sample = std::clamp(sample, -1.0f, 1.0f);
        samples[i] = static_cast<int16_t>(sample * 26000);
    }

    return samples;
}
