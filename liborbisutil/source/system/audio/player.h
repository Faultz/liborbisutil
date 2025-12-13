#pragma once

// Audio player - standard includes provided by stdafx.h
// Depends on: wav

#include "wav.h"

namespace liborbisutil
{
	namespace audio
	{
		// Forward-declare player to be used in track if needed
		class player;

		class track
		{
		public:
			// New: Define the size of each audio chunk to be read from the file.
			// This determines the memory usage per stream. 4096 frames is a reasonable default.
			static constexpr size_t CHUNK_FRAMES = 4096;

			track() = default;
			~track();

			// Public API - largely unchanged
			bool load(const std::string& filename);
			void play();
			void pause();
			void stop();
			void set_position(size_t frame_position);
			void set_volume(float volume);
			void set_loop(bool loop);
			bool is_playing() const;

			std::string fileName;

		private:
			friend class player; // Allow player to access read_samples

			// This is now the core method for providing samples to the player.
			// It reads from internal buffers and triggers decoding new chunks as needed.
			size_t read_samples(float* buffer, size_t frames_to_read);

			// New: Core streaming function to read and process the next block from the file.
			bool decode_chunk();

			// Helper methods for format conversion
			void convert_pcm16(const int16_t* input, size_t frames, uint16_t channels, std::vector<float>& output);
			void convert_pcm24(const uint8_t* input, size_t frames, uint16_t channels, std::vector<float>& output);
			void convert_pcm32(const int32_t* input, size_t frames, uint16_t channels, std::vector<float>& output);
			void convert_float(const float* input, size_t frames, uint16_t channels, std::vector<float>& output);
			void convert_channels(const std::vector<float>& input, uint16_t in_channels, std::vector<float>& output);

			// Thread-safe state variables
			std::atomic<bool> playing = false;
			std::atomic<bool> loop = false;
			std::atomic<float> volume = 1.0f;

			// File and stream management
			wave file;
			size_t current_frame_position = 0;
			uintptr_t data_start_offset = 0;
			size_t data_size = 0;

			// Source audio format details
			uint16_t source_format;
			uint32_t source_sample_rate;
			uint16_t source_channels;
			uint16_t source_bps;

			// Buffers for streaming and processing
			std::vector<char> raw_buffer;        // Holds the raw data read from disk
			std::vector<float> decoded_buffer;   // Holds float-converted data
			std::vector<float> resampled_buffer; // Holds final, resampled stereo data ready for the player
			size_t resampled_buffer_read_pos = 0;   // Current read position in resampled_buffer
		};

		class player
		{
		public:
			enum init_flags
			{
				main_only = 0,
				pad = 1,
			};

			player() = default;
			~player();

			bool initialize(int flags = main_only);
			void shutdown();

			void add_track(std::shared_ptr<track> track);
			void set_master_volume(float volume);

			std::shared_ptr<track> get_track(const std::string& filename);
			bool has_track(const std::string& filename) const;

			std::vector<std::shared_ptr<track>> get_tracks() const
			{
				return tracks;
			}

		private:
			void audio_thread_main();

			std::thread audio_thread;
			std::atomic<bool> running = false;
			std::mutex mutex;

			int main_handle = -1;
			int pad_handle = -1;

			std::vector<std::shared_ptr<track>> tracks;
			std::atomic<float> master_volume = 1.0f;
		};
	}
}