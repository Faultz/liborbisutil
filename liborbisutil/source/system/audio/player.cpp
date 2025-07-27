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

		// New: Destructor to ensure file is closed
		track::~track()
		{
			// The WaveFile object will close the file in its own destructor
		}

		bool track::load(const std::string& filename)
		{
			if (!file.open(filename)) return false;
			if (!file.read_header()) return false;

			// Store source audio format details
			source_format = file.get_format();
			source_sample_rate = file.get_sample_rate();
			source_channels = file.get_channels();
			source_bps = file.format.bits_per_sample;
			data_start_offset = file.data.chunk_offset;
			data_size = file.data.chunk_size;

			// Check for valid format
			if (source_bps == 0 || source_channels == 0)
			{
				LOG_ERROR("Invalid audio file header: %s\n", filename.c_str());
				return false;
			}

			// Allocate buffers for processing one chunk at a time
			const size_t raw_chunk_size = CHUNK_FRAMES * source_channels * (source_bps / 8);
			raw_buffer.resize(raw_chunk_size);
			decoded_buffer.resize(CHUNK_FRAMES * source_channels);

			// Mark the processed buffer as empty so the first read triggers a decode
			resampled_buffer_read_pos = 0;
			resampled_buffer.clear();

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
			// Reset position to the beginning of the stream
			set_position(0);
		}

		// set_position now seeks the frame position in the stream
		void track::set_position(size_t frame_position)
		{
			const size_t bytes_per_frame = source_channels * (source_bps / 8);
			const size_t total_frames = data_size / bytes_per_frame;

			// Clamp position to valid range
			this->current_frame_position = std::min(frame_position, total_frames);

			// Invalidate the current buffer to force a read from the new file position
			resampled_buffer_read_pos = 0;
			resampled_buffer.clear();
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

		// The get_samples method is removed as it's not compatible with streaming.
		// const std::vector<float>& track::get_samples() const;

		void resample_audio(const std::vector<float>& input, uint32_t in_rate,
			std::vector<float>& output, uint32_t out_rate,
			uint16_t channels) {
			if (in_rate == out_rate) {
				output = input;
				return;
			}

			const double ratio = static_cast<double>(out_rate) / in_rate;
			const size_t in_frames = input.size() / channels;
			const size_t out_frames = static_cast<size_t>(in_frames * ratio);
			output.resize(out_frames * channels);

			for (size_t i = 0; i < out_frames; ++i) {
				const double in_frame_idx = i / ratio;
				const size_t base_frame = static_cast<size_t>(in_frame_idx);
				const double fraction = in_frame_idx - base_frame;

				for (uint16_t c = 0; c < channels; ++c) {
					const size_t base_idx = base_frame * channels + c;
					const size_t next_idx = (base_frame + 1) * channels + c;

					const float s1 = input[std::min(base_idx, input.size() - 1)];
					const float s2 = input[std::min(next_idx, input.size() - 1)];

					output[i * channels + c] = s1 + fraction * (s2 - s1);
				}
			}
		}

		// read_samples now reads from internal, refilling buffers from the file as needed.
		size_t track::read_samples(float* buffer, size_t frames_to_read) {
			if (!playing.load()) return 0;

			const size_t samples_to_read = frames_to_read * TARGET_CHANNELS;
			size_t samples_written = 0;
			const float vol = volume.load();

			while (samples_written < samples_to_read) {
				// If our processed buffer is exhausted, decode a new chunk from the file.
				if (resampled_buffer_read_pos >= resampled_buffer.size()) {
					if (!decode_chunk()) {
						// Decoding failed, which means end of stream and not looping.
						playing = false;
						break; // Exit the while loop
					}
				}

				// Copy available samples from our internal buffer to the output buffer.
				const size_t remaining_in_buffer = resampled_buffer.size() - resampled_buffer_read_pos;
				const size_t samples_to_copy_now = std::min(remaining_in_buffer, samples_to_read - samples_written);

				for (size_t i = 0; i < samples_to_copy_now; i++) {
					buffer[samples_written + i] += resampled_buffer[resampled_buffer_read_pos + i] * vol;
				}

				resampled_buffer_read_pos += samples_to_copy_now;
				samples_written += samples_to_copy_now;
			}

			// This part is important: if playback stopped mid-buffer, the remainder of the player's
			// buffer must be zeroed to prevent leftover audio from playing.
			if (samples_written < samples_to_read) {
				std::memset(buffer + samples_written, 0, (samples_to_read - samples_written) * sizeof(float));
			}

			return samples_written / TARGET_CHANNELS;
		}

		// New: This function reads and processes the next chunk of audio from the file.
		bool track::decode_chunk() {
			const size_t bytes_per_frame = source_channels * (source_bps / 8);

			// Check if we're at the end of the data chunk.
			if ((current_frame_position * bytes_per_frame) >= data_size) {
				if (loop.load()) {
					current_frame_position = 0; // Seek back to the beginning for looping.
				}
				else {
					return false; // End of stream.
				}
			}

			// Read a raw chunk from the file.
			const size_t file_byte_pos = data_start_offset + (current_frame_position * bytes_per_frame);
			const size_t remaining_bytes_in_file = data_size - (current_frame_position * bytes_per_frame);
			const size_t bytes_to_read = std::min(remaining_bytes_in_file, raw_buffer.size());

			size_t bytes_actually_read;
			if (!file.read_data(file_byte_pos, raw_buffer.data(), bytes_to_read, &bytes_actually_read) || bytes_actually_read == 0) {
				return false;
			}

			const size_t frames_read = bytes_actually_read / bytes_per_frame;
			current_frame_position += frames_read;

			// Resize decoded buffer to match the number of frames we actually read.
			decoded_buffer.resize(frames_read * source_channels);

			// Convert the raw data to normalized float format.
			switch (source_format) {
			case 1: // PCM
				switch (source_bps) {
				case 16: convert_pcm16(reinterpret_cast<int16_t*>(raw_buffer.data()), frames_read, source_channels, decoded_buffer); break;
				case 24: convert_pcm24((uint8_t*)raw_buffer.data(), frames_read, source_channels, decoded_buffer); break;
				case 32: convert_pcm32(reinterpret_cast<int32_t*>(raw_buffer.data()), frames_read, source_channels, decoded_buffer); break;
				default: return false;
				}
				break;
			case 3: // Float
				convert_float(reinterpret_cast<float*>(raw_buffer.data()), frames_read, source_channels, decoded_buffer); break;
			default: return false;
			}

			// Handle channel conversion
			std::vector<float> channel_converted_chunk;
			convert_channels(decoded_buffer, source_channels, channel_converted_chunk);

			// Resample to the target rate. The resampled_buffer is the final destination.
			resample_audio(channel_converted_chunk, source_sample_rate, resampled_buffer, TARGET_SAMPLE_RATE, TARGET_CHANNELS);

			// Reset the read position for the newly filled buffer.
			resampled_buffer_read_pos = 0;

			return !resampled_buffer.empty();
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

			const size_t in_frames = input.size() / in_channels;
			output.resize(in_frames * TARGET_CHANNELS);

			if (in_channels == 1) {
				// Mono to stereo
				for (size_t i = 0; i < in_frames; i++) {
					output[i * 2] = input[i];
					output[i * 2 + 1] = input[i];
				}
			}
			else {
				// Downmix multi-channel to stereo
				for (size_t i = 0; i < in_frames; i++) {
					float mix = 0.0f;
					for (uint16_t c = 0; c < in_channels; c++) {
						mix += input[i * in_channels + c];
					}
					mix /= in_channels;
					output[i * TARGET_CHANNELS] = mix;
					output[i * TARGET_CHANNELS + 1] = mix;
				}
			}
		}

		// The player class implementation below remains unchanged as its logic
		// for mixing and outputting audio is independent of how the track gets its samples.

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
				0, 256, /*48000*/48000, SCE_AUDIO_OUT_PARAM_FORMAT_S16_STEREO);
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

				bool playing = false;
				// Mix tracks into stereo buffer
				{
					std::lock_guard<std::mutex> lock(mutex);
					for (auto& track : tracks) {

						if (track->is_playing())
						{
							playing = true;

							track->read_samples(stereo_mix.data(), BUFFER_FRAMES);
						}
					}

					/// If no tracks are playing, idle
					if (!playing) {
						sceKernelUsleep(1000);
						continue;
					}
				}

				//float muiltichannel_buffer[BUFFER_FRAMES * 8 * sizeof(float)];
				// Duplicate the mono channel to all 8 channels
				//for (int i = 0; i < stereo_size; ++i) {
				// 
				//	// Extract left and right samples for the current frame
				//	float left_sample = stereo_mix[i * 2];
				//	float right_sample = stereo_mix[i * 2 + 1];

				//	// Downmix to mono by averaging the left and right channels
				//	float mono_sample = (left_sample + right_sample) / 2.0f;

				//	// Duplicate the mono sample to all 8 channels for the current frame
				//	for (int j = 0; j < 8; ++j) {
				//		muiltichannel_buffer[i * 8 + j] = mono_sample;
				//	}
				//}

				for (size_t i = 0; i < stereo_size; i++) {
					float sample = std::clamp(stereo_mix[i], -1.0f, 1.0f);
					stereo_output[i] = static_cast<int16_t>(sample * 32767.0f);
				}

				// Submit to main output
				sceAudioOutOutput(main_handle, stereo_output.data());

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
