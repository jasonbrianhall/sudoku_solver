#ifndef GAME_SOUND_H
#define GAME_SOUND_H

#include <vector>
#include <utility>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#endif

// Best-effort audio feedback for the Game app.
//
// The original C++/CLI code called the Win32 ::Beep(freq, duration) API,
// which plays an actual tone at a given frequency - something Windows
// alone provides for free. There's no equivalent zero-dependency API on
// Linux/macOS, so on those platforms we fall back to the system bell
// (wxBell(), wired up by the caller) for a single short blip instead of
// reproducing exact tunes. This keeps the app dependency-free; if you
// want the original tunes cross-platform, swap this for a small library
// like SDL2_mixer or miniaudio and play short generated sine waves.
namespace GameSound {

inline void Tone(int freqHz, int durationMs) {
#ifdef _WIN32
    ::Beep(freqHz, durationMs);
#else
    (void)freqHz;
    (void)durationMs;
    std::this_thread::sleep_for(std::chrono::milliseconds(durationMs / 4));
#endif
}

// freqs[i] == 0 means "silence for durations[i] ms" (a rest).
inline void PlaySequenceAsync(std::vector<std::pair<int, int>> notes) {
    std::thread([notes = std::move(notes)]() {
        for (auto& n : notes) {
            if (n.first == 0)
                std::this_thread::sleep_for(std::chrono::milliseconds(n.second));
            else
                Tone(n.first, n.second);
        }
    }).detach();
}

inline void PlayAsync(int freqHz, int durationMs) {
    std::thread([freqHz, durationMs]() { Tone(freqHz, durationMs); }).detach();
}

} // namespace GameSound

#endif // GAME_SOUND_H
