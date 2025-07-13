#include "player.h"
#include "../logger.h"
#include "../resolve.h"
#include <audioout.h>

namespace liborbisutil
{
	namespace audio
	{
		constexpr uint32_t TARGET_SAMPLE_RATE = 48000;
		constexpr uint16_t TARGET_CHANNELS = 2;

		bool track::load(const std::string& filename)
		{
			if (!file.open(filename)) return false;
			if (!decode()) return false;
		
			fileName = filename;
			return true;
		}
		void track::play()
		{
			playing = true;
		}
		void track::pause()
		{
			playing = false;
		}
		void track::stop()
		{
			playing = false;
			position = 0;
		}
		void track::set_position(size_t position)
		{
			this->position = position;
		}
		void track::set_volume(float volume)
		{
			this->volume = std::clamp(volume, 0.0f, 1.0f);
		}
		void track::set_loop(bool loop)
		{
			this->loop = loop;
		}
		bool track::is_playing() const
		{
			return playing;
		}
		const std::vector<float>& track::get_samples() const
		{
			return samples;
		}

		void resample_audio(const std::vector<float>& input, uint32_t in_rate,
			std::vector<float>& output, uint32_t out_rate,
			uint16_t channels) {
			const double ratio = static_cast<double>(in_rate) / out_rate;
			const size_t out_size = static_cast<size_t>(input.size() * ratio);
			output.resize(out_size);

			for (size_t i = 0; i < out_size; i += channels) {
				const double read_pos = i * ratio;
				const size_t base = static_cast<size_t>(read_pos);
				const double fraction = read_pos - base;

				for (uint16_t c = 0; c < channels; c++) {
					const float s1 = input[std::min(base + c, input.size() - 1)];
					const float s2 = input[std::min(base + channels + c, input.size() - 1)];
					output[i + c] = s1 + fraction * (s2 - s1);
				}
			}
		}

		size_t track::read_samples(float* buffer, size_t frames_to_read) {
			if (!playing.load()) return 0;

			const size_t samples_to_read = frames_to_read * TARGET_CHANNELS;
			size_t samples_read = 0;
			const float vol = volume.load();

			while (samples_read < samples_to_read) {
				if (position >= samples.size()) {
					//if (!loop.load()) {
					//	// Zero remaining buffer space
					//	std::memset(buffer + samples_read, 0,
					//		(samples_to_read - samples_read) * sizeof(float));
					//	break;
					//}
					position = 0;

					// Stop playback if not looping
					if (!loop.load())
						playing = false;
				}

				const size_t remaining = samples.size() - position;
				const size_t to_copy = std::min(remaining, samples_to_read - samples_read);

				for (size_t i = 0; i < to_copy; i++) {
					buffer[samples_read + i] += samples[position + i] * vol;
				}

				samples_read += to_copy;
				position += to_copy;
			}

			return samples_read / TARGET_CHANNELS;
		}

		bool track::decode() {
			if (!file.read_header()) return false;

			const auto format = file.get_format();
			const auto sample_rate = file.get_sample_rate();
			const auto channels = file.get_channels();
			const auto bps = file.format.bits_per_sample;

			// Read raw audio data
			const uintptr_t offset = file.data.chunk_offset;
			const size_t data_size = file.data.chunk_size;
			std::vector<char> raw_data(data_size);
			size_t read;
			if (!file.read_data(offset, raw_data.data(), data_size, &read) || read != data_size) {
				return false;
			}

			// Convert to normalized float format
			const size_t frame_count = data_size / (channels * (bps / 8));
			std::vector<float> raw_samples(frame_count * channels);

			switch (format) {
			case 1: // PCM
				switch (bps) {
				case 16:
					convert_pcm16(reinterpret_cast<int16_t*>(raw_data.data()),
						frame_count, channels, raw_samples);
					break;
				case 24:
					convert_pcm24((uint8_t*)raw_data.data(), frame_count, channels, raw_samples);
					break;
				case 32:
					convert_pcm32(reinterpret_cast<int32_t*>(raw_data.data()),
						frame_count, channels, raw_samples);
					break;
				default: return false;
				}
				break;

			case 3: // Float
				convert_float(reinterpret_cast<float*>(raw_data.data()),
					frame_count, channels, raw_samples);
				break;

			default: return false;
			}

			// Handle channel conversion
			std::vector<float> channel_converted;
			convert_channels(raw_samples, channels, channel_converted);

			// Resample if needed
			if (sample_rate != TARGET_SAMPLE_RATE) {
				resample_audio(channel_converted, sample_rate,
					samples, TARGET_SAMPLE_RATE, TARGET_CHANNELS);
			}
			else {
				samples = std::move(channel_converted);
			}

			return true;
		}

		void track::convert_pcm16(const int16_t* input, size_t frames,
			uint16_t channels, std::vector<float>& output) {
			const float scale = 1.0f / 32768.0f;
			for (size_t i = 0; i < frames * channels; i++) {
				output[i] = input[i] * scale;
			}
		}

		void track::convert_pcm24(const uint8_t* input, size_t frames,
			uint16_t channels, std::vector<float>& output) {
			const float scale = 1.0f / 8388608.0f;
			for (size_t i = 0; i < frames; i++) {
				for (uint16_t c = 0; c < channels; c++) {
					const size_t offset = (i * channels + c) * 3;
					int32_t sample = (input[offset + 2] << 24) |
						(input[offset + 1] << 16) |
						(input[offset] << 8);
					output[i * channels + c] = (sample >> 8) * scale;
				}
			}
		}

		void track::convert_pcm32(const int32_t* input, size_t frames,
			uint16_t channels, std::vector<float>& output) {
			const float scale = 1.0f / 2147483648.0f;
			for (size_t i = 0; i < frames * channels; i++) {
				output[i] = input[i] * scale;
			}
		}

		void track::convert_float(const float* input, size_t frames,
			uint16_t channels, std::vector<float>& output) {
			for (size_t i = 0; i < frames * channels; i++) {
				output[i] = std::clamp(input[i], -1.0f, 1.0f);
			}
		}

		void track::convert_channels(const std::vector<float>& input,
			uint16_t in_channels,
			std::vector<float>& output) {
			if (in_channels == TARGET_CHANNELS) {
				output = input;
				return;
			}

			output.resize((input.size() / in_channels) * TARGET_CHANNELS);

			if (in_channels == 1) {
				// Mono to stereo
				for (size_t i = 0; i < input.size(); i++) {
					output[i * 2] = input[i];
					output[i * 2 + 1] = input[i];
				}
			}
			else {
				for (size_t i = 0; i < output.size() / TARGET_CHANNELS; i++) {
					float mix = 0.0f;
					for (uint16_t c = 0; c < in_channels; c++) {
						mix += input[i * in_channels + c];
					}
					mix /= in_channels;
					output[i * 2] = mix;
					output[i * 2 + 1] = mix;
				}
			}
		}

		player::~player()
		{
			shutdown();
		}

		bool player::initialize(int flags)
		{
			int currentUserID = 0;
			SceUserServiceLoginUserIdList list;
			sceUserServiceGetLoginUserIdList(&list);
			for (int i = 0; i < 4; i++)
			{
				if (list.userId[i] != -1)
				{
					currentUserID = list.userId[i];
					break;
				}
			}

			main_handle = sceAudioOutOpen(SCE_USER_SERVICE_USER_ID_SYSTEM, SCE_AUDIO_OUT_PORT_TYPE_MAIN,
				0, 256, /*48000*/48000, SCE_AUDIO_OUT_PARAM_FORMAT_FLOAT_8CH);
			if (main_handle < 0)
			{
				LOG_ERROR("sceAudioOutOpen main failed: 0x%08X\n", main_handle);

				main_handle = sceAudioOutOpen(currentUserID, SCE_AUDIO_OUT_PORT_TYPE_BGM,
					0, 256, 48000, SCE_AUDIO_OUT_PARAM_FORMAT_FLOAT_8CH);
				if (main_handle < 0)
				{
					LOG_ERROR("sceAudioOutOpen bgm failed: 0x%08X\n", main_handle);
					return false;
				}
			}


			if (flags & pad)
			{
				pad_handle = sceAudioOutOpen(currentUserID, SCE_AUDIO_OUT_PORT_TYPE_PADSPK,
					0, 256, 48000, SCE_AUDIO_OUT_PARAM_FORMAT_S16_MONO);
				if (pad_handle < 0)
				{
					LOG_ERROR("sceAudioOutOpen pad failed: 0x%08X\n", pad_handle);
					return false;
				}
			}

			running = true;
			audio_thread = std::thread(&player::audio_thread_main, this);
			int cpu = 5;
			const static uint cAllCoresMask = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5);
			uint mask = (cpu >= 0) ? (1 << cpu) : cAllCoresMask;

			auto native_handle = audio_thread.native_handle();
			scePthreadSetaffinity(native_handle, mask);
			SceKernelSchedParam p = { SCE_KERNEL_PRIO_FIFO_DEFAULT - 15 };
			scePthreadSetschedparam(native_handle, SCE_KERNEL_SCHED_FIFO, &p);

			return true;
		}
		void player::shutdown()
		{
			running = false;
			if (audio_thread.joinable())
				audio_thread.join();

			sceAudioOutOutput(main_handle, nullptr);

			auto res = sceAudioOutClose(main_handle);
			LOG_ERROR("sceAudioOutClose failed: 0x%08X\n", res);

			main_handle = -1;

			sceAudioOutOutput(pad_handle, nullptr);

			res = sceAudioOutClose(pad_handle);
			LOG_ERROR("sceAudioOutClose failed: 0x%08X\n", res);

			pad_handle = -1;
		}

		void player::add_track(std::shared_ptr<track> track)
		{
			std::lock_guard<std::mutex> lock(mutex);
			tracks.push_back(track);
		}

		void player::set_master_volume(float volume)
		{
			master_volume = std::clamp(volume, 0.0f, 1.0f);

			int vol[8];

			for (int i = 0; i < 8; i++)
				vol[i] = (int)((float)SCE_AUDIO_OUT_VOLUME_0DB * volume);

			sceAudioOutSetVolume(main_handle, SCE_AUDIO_VOLUME_FLAG_FL_CH | SCE_AUDIO_VOLUME_FLAG_FR_CH
				| SCE_AUDIO_VOLUME_FLAG_CNT_CH | SCE_AUDIO_VOLUME_FLAG_LFE_CH
				| SCE_AUDIO_VOLUME_FLAG_RL_CH | SCE_AUDIO_VOLUME_FLAG_RR_CH
				| SCE_AUDIO_VOLUME_FLAG_BL_CH | SCE_AUDIO_VOLUME_FLAG_BR_CH, vol);
			sceAudioOutSetVolume(pad_handle, SCE_AUDIO_VOLUME_FLAG_FL_CH | SCE_AUDIO_VOLUME_FLAG_FR_CH
				| SCE_AUDIO_VOLUME_FLAG_CNT_CH | SCE_AUDIO_VOLUME_FLAG_LFE_CH
				| SCE_AUDIO_VOLUME_FLAG_RL_CH | SCE_AUDIO_VOLUME_FLAG_RR_CH
				| SCE_AUDIO_VOLUME_FLAG_BL_CH | SCE_AUDIO_VOLUME_FLAG_BR_CH, vol);
		}

		std::shared_ptr<track> player::get_track(const std::string& filename)
		{
			for (auto& track : tracks)
			{
				if (track->fileName == filename)
					return track;
			}
			return nullptr;
		}

		bool player::has_track(const std::string& filename) const
		{
			for (auto& track : tracks)
			{
				if (track->fileName == filename)
					return true;
			}
			return false;
		}

		constexpr static size_t BUFFER_FRAMES = 256;
		// Player implementation
		void player::audio_thread_main() {
			const size_t stereo_size = BUFFER_FRAMES * 2;
			const size_t mono_size = BUFFER_FRAMES;

			std::vector<float> stereo_mix(stereo_size, 0.0f);
			std::vector<int16_t> stereo_output(stereo_size);
			std::vector<int16_t> mono_output(mono_size);

			while (running) {
				// Clear buffers
				std::fill(stereo_mix.begin(), stereo_mix.end(), 0.0f);

				// Mix tracks into stereo buffer
				{
					std::lock_guard<std::mutex> lock(mutex);
					for (auto& track : tracks) {
						if (track->is_playing())
						{
							track->read_samples(stereo_mix.data(), BUFFER_FRAMES);
						}
					}
				}

				float muiltichannel_buffer[BUFFER_FRAMES * 8 * sizeof(float)];

				// Duplicate the mono channel to all 8 channels
				for (int i = 0; i < stereo_size; ++i) {
					// Extract left and right samples for the current frame
					float left_sample = stereo_mix[i * 2];
					float right_sample = stereo_mix[i * 2 + 1];

					// Downmix to mono by averaging the left and right channels
					float mono_sample = (left_sample + right_sample) / 2.0f;

					// Duplicate the mono sample to all 8 channels for the current frame
					for (int j = 0; j < 8; ++j) {
						muiltichannel_buffer[i * 8 + j] = mono_sample;
					}
				}

				// Submit to main output
				sceAudioOutOutput(main_handle, muiltichannel_buffer);

				// Convert to 16-bit stereo
				//for (size_t i = 0; i < stereo_size; i++) {
				//	float sample = std::clamp(stereo_mix[i], -1.0f, 1.0f);
				//	stereo_output[i] = static_cast<int16_t>(sample * 32767.0f);
				//}

				//// Submit to main output
				//sceAudioOutOutput(main_handle, stereo_output.data());

				// Handle pad output if connected
				// Downmix to mono
				for (size_t i = 0; i < BUFFER_FRAMES; i++) {
					float left = stereo_mix[i * 2];
					float right = stereo_mix[i * 2 + 1];
					float mono = (left + right) * 0.7071f; // -3dB to prevent clipping
					mono = std::clamp(mono, -1.0f, 1.0f);
					mono_output[i] = static_cast<int16_t>(mono * 32767.0f);
				}

				// Submit to pad
				if (pad_handle >= 0)
				{
					sceAudioOutOutput(pad_handle, mono_output.data());
				}
			}
		}

	}
}