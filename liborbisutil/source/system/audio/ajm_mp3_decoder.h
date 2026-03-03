#pragma once

#include <ajm.h>
#include <ajm/mp3_decoder.h>
#include <cstdint>
#include <cstddef>

namespace ajm
{
    // AJM-accelerated MP3 decoder with optional hardware resampling.
    // One instance per audio track — maintains per-stream decode state. Subject to change.
    class mp3_decoder
    {
    public:
        // Initialize the decoder.
        // max_channels : 1 (mono) or 2 (stereo) — must match the MP3 source.
        // source_rate  : native sample rate of the MP3 (e.g. 44100, 48000).
        // target_rate  : desired output rate; pass 0 or same as source to disable resampling.
        void create();
        bool initialize(uint32_t max_channels = 2,
                        uint32_t source_rate  = 0,
                        uint32_t target_rate  = 0);

        void finalize();

        // Decode a chunk of MP3 bitstream to S16 PCM.
        // Returns bytes of PCM produced; sets *bytes_consumed_out to input bytes consumed.
        // Output is interleaved S16 at the target rate (or source rate if not resampling).
        uint32_t decode(const void* mp3_data, size_t mp3_size,
                        int16_t*    pcm_out,  size_t pcm_out_size,
                        size_t*     bytes_consumed_out);

        bool is_initialized() const { return m_initialized; }

    private:
        SceAjmContextId  m_context     = SCE_AJM_CONTEXT_INVALID;
        SceAjmInstanceId m_instance    = SCE_AJM_INSTANCE_INVALID;
        bool             m_initialized = false;
    };
}
