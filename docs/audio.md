# Audio Module

WAV file playback and audio streaming system.

## Audio Player

The `player` class manages audio output and tracks.

```cpp
#include <liborbisutil.h>

// Create and initialize player
liborbisutil::audio::player audioPlayer;

// Initialize with main output only
audioPlayer.initialize(liborbisutil::audio::player::main_only);

// Or with controller speaker support
audioPlayer.initialize(liborbisutil::audio::player::pad);
```

### Managing Tracks

```cpp
// Create and load a track
auto track = std::make_shared<liborbisutil::audio::track>();
if (track->load("/app0/audio/music.wav")) {
    audioPlayer.add_track(track);
}

// Control playback
track->play();
track->pause();
track->stop();

// Set volume (0.0 - 1.0)
track->set_volume(0.8f);

// Enable looping
track->set_loop(true);

// Seek to position (frame number)
track->set_position(44100);  // 1 second at 44.1kHz

// Check if playing
if (track->is_playing()) {
    // Track is active
}
```

### Master Volume

```cpp
// Set global volume
audioPlayer.set_master_volume(0.5f);
```

### Cleanup

```cpp
audioPlayer.shutdown();
```

## Wave File Class

Low-level WAV file handling.

```cpp
wave wavFile;

if (wavFile.open("/app0/audio/sound.wav")) {
    // Read header info
    wavFile.read_header();
    
    // Get format info
    int format = wavFile.get_format();
    int sampleRate = wavFile.get_sample_rate();
    int channels = wavFile.get_channels();
    
    // Check if mono
    if (wavFile.is_mono()) {
        // Single channel audio
    }
    
    // Get duration in milliseconds
    uint32_t duration = wavFile.get_duration();
    
    // Read audio data
    char buffer[4096];
    size_t bytesRead;
    wavFile.read_data(0, buffer, sizeof(buffer), &bytesRead);
    
    // Read specific sample
    wavFile.read_sample(sampleIndex, buffer, bufferSize, &bytesRead);
    
    wavFile.close();
}
```

## Supported Formats

The track class supports various WAV formats:

- **PCM 16-bit** - Standard CD-quality audio
- **PCM 24-bit** - High-resolution audio
- **PCM 32-bit** - Studio-quality audio
- **Float 32-bit** - Floating-point samples

Audio is automatically:
- Converted to stereo if mono
- Resampled if needed
- Volume-adjusted

## Example: Background Music System

```cpp
class MusicSystem {
    liborbisutil::audio::player player;
    std::shared_ptr<liborbisutil::audio::track> bgm;
    
public:
    void init() {
        player.initialize();
    }
    
    void play_bgm(const std::string& filename) {
        if (bgm) {
            bgm->stop();
        }
        
        bgm = std::make_shared<liborbisutil::audio::track>();
        if (bgm->load(filename)) {
            bgm->set_loop(true);
            bgm->set_volume(0.7f);
            player.add_track(bgm);
            bgm->play();
        }
    }
    
    void set_volume(float vol) {
        if (bgm) bgm->set_volume(vol);
    }
    
    void shutdown() {
        player.shutdown();
    }
};
```

## Example: Sound Effects

```cpp
class SoundManager {
    liborbisutil::audio::player player;
    std::vector<std::shared_ptr<liborbisutil::audio::track>> sounds;
    
public:
    void play_sfx(const std::string& filename) {
        auto sfx = std::make_shared<liborbisutil::audio::track>();
        if (sfx->load(filename)) {
            sfx->set_loop(false);
            player.add_track(sfx);
            sfx->play();
            sounds.push_back(sfx);
        }
    }
    
    void cleanup_finished() {
        sounds.erase(
            std::remove_if(sounds.begin(), sounds.end(),
                [](auto& s) { return !s->is_playing(); }),
            sounds.end()
        );
    }
};
```

## WAV Header Structure

```cpp
struct WAV_HEADER {
    uint8_t  RIFF[4];        // "RIFF"
    uint32_t ChunkSize;
    uint8_t  WAVE[4];        // "WAVE"
    uint8_t  fmt[4];         // "fmt "
    uint32_t Subchunk1Size;
    uint16_t AudioFormat;    // 1=PCM, 3=Float
    uint16_t NumOfChan;
    uint32_t SamplesPerSec;
    uint32_t bytesPerSec;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    uint8_t  Subchunk2ID[4]; // "data"
    uint32_t Subchunk2Size;
};
```
