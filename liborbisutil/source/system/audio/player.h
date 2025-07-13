#pragma once

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <thread>
#include <mutex>

#include <atomic>

#include "wav.h"

namespace liborbisutil
{
	namespace audio
	{
		enum type
		{
			bgm = 1 << 0,
			main = 1 << 1,
			pad = 1 << 2
		};

		class track
		{
		public:
			track() : volume(1.0f), loop(false), position(0), playing(false) {}

			bool load(const std::string& filename);

			void play();
			void pause();
			void stop();

			void set_position(size_t position);

			void set_volume(float volume);
			void set_loop(bool loop);

			bool is_playing() const;
			size_t get_position() const { return position; }
			size_t get_length() const { return samples.size(); }
			const std::vector<float>& get_samples() const;
			float get_volume() const { return volume; }

			size_t read_samples(float* buffer, size_t frames_to_read);
			
			std::string fileName;
		private:
			bool decode();

			void convert_pcm16(const int16_t* input, size_t frames,
				uint16_t channels, std::vector<float>& output);
			void convert_pcm24(const uint8_t* input, size_t frames,
				uint16_t channels, std::vector<float>& output);
			void convert_pcm32(const int32_t* input, size_t frames,
				uint16_t channels, std::vector<float>& output);
			void convert_float(const float* input, size_t frames,
				uint16_t channels, std::vector<float>& output);
			void convert_8ch_to_stereo(const std::vector<float>& input, size_t frames,
				std::vector<float>& output);
			void convert_channels(const std::vector<float>& input,
				uint16_t in_channels,
				std::vector<float>& output);

			wave file;
			std::vector<float> samples;
			std::atomic<float> volume;
			std::atomic<bool> loop;
			std::atomic<size_t> position;
			std::atomic<bool> playing;
		};

		class player
		{
		public:
			player() : main_handle(-1), pad_handle(-1), master_volume(1.0f), running(false) {}
			~player();

			bool initialize(int flags = bgm);
			void shutdown();
			void add_track(std::shared_ptr<track> track);
			void set_master_volume(float volume);

			std::shared_ptr<track> get_track(const std::string& filename);

			bool has_track(const std::string& filename) const;
			bool is_initialized() const { return main_handle >= 0; }

			std::mutex mutex;
			std::vector<std::shared_ptr<track>> tracks;
		private:
			void audio_thread_main();

			int main_handle;
			int pad_handle;
			std::atomic<float> master_volume;
			std::thread audio_thread;
			std::atomic<bool> running;
		};
	}
}