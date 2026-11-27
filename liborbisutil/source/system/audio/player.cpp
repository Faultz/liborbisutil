#include "../../stdafx.h"
#include "player.h"
#include "../logger/logger.h"
#include "../resolve/resolve.h"
#include <audioout.h>
#include <voice.h>

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
			if (mp3_file)
			{
				fclose(mp3_file);
				mp3_file = nullptr;
			}
			mp3_dec.finalize();
		}

		bool track::load(const std::string& filename)
		{
			// Detect MP3 by file extension
			const bool is_mp3 = filename.size() >= 4 &&
				(filename.substr(filename.size() - 4) == ".mp3" ||
				 filename.substr(filename.size() - 4) == ".MP3");

			if (is_mp3)
			{
				// Close any previously open file
				if (mp3_file) { fclose(mp3_file); mp3_file = nullptr; }

				mp3_file = fopen(filename.c_str(), "rb");
				if (!mp3_file)
				{
					LOG_ERROR("Failed to open MP3: %s\n", filename.c_str());
					return false;
				}

				mp3_dec.create();

				// Skip any ID3v2 tag so the probe lands on actual MP3 frame data.
				// ID3v2 header: "ID3" + 2 version bytes + flags + 4-byte syncsafe size.
				uint8_t id3_hdr[10] = {};
				if (fread(id3_hdr, 1, 10, mp3_file) == 10 &&
					id3_hdr[0] == 0x49 && id3_hdr[1] == 0x44 && id3_hdr[2] == 0x33) // "ID3"
				{
					// Syncsafe integer: each byte contributes 7 bits (MSB always 0).
					const uint32_t tag_size =
						((uint32_t)(id3_hdr[6] & 0x7f) << 21) |
						((uint32_t)(id3_hdr[7] & 0x7f) << 14) |
						((uint32_t)(id3_hdr[8] & 0x7f) <<  7) |
						 (uint32_t)(id3_hdr[9] & 0x7f);
					fseek(mp3_file, 10 + (long)tag_size, SEEK_SET);
				}
				else
				{
					fseek(mp3_file, 0, SEEK_SET); // not ID3v2 - rewind and probe from start
				}

				// Record start of audio data (after ID3 tag, if any) for loop/seek.
				data_start_offset = static_cast<uintptr_t>(ftell(mp3_file));

				// Read a probe buffer to find the first valid MPEG Layer III frame header.
				constexpr size_t PROBE_SIZE = 4096;
				uint8_t probe[PROBE_SIZE];
				const size_t probe_read = fread(probe, 1, PROBE_SIZE, mp3_file);

				// MPEG Layer III header parser - no AJM dependency, safe before sceAjmInitialize().
				// Sample rate table: [mpeg_idx][sr_idx] where 0=MPEG1, 1=MPEG2, 2=MPEG2.5
				static const uint32_t sr_table[3][3] = {
					{ 44100, 48000, 32000 }, // MPEG 1
					{ 22050, 24000, 16000 }, // MPEG 2
					{ 11025, 12000,  8000 }, // MPEG 2.5
				};
				uint32_t native_rate     = 44100;
				uint32_t native_channels = 2;
				bool parsed = false;
				for (size_t scan = 0; scan + 3 < probe_read && !parsed; ++scan)
				{
					if (probe[scan] != 0xFF || (probe[scan + 1] & 0xE0) != 0xE0) continue;
					const int mpeg_ver = (probe[scan + 1] >> 3) & 0x3; // 3=MPEG1, 2=MPEG2, 0=MPEG2.5, 1=rsvd
					const int layer    = (probe[scan + 1] >> 1) & 0x3; // must be 1 (Layer III)
					if (layer != 1 || mpeg_ver == 1) continue;
					const int sr_idx   = (probe[scan + 2] >> 2) & 0x3;
					if (sr_idx == 3) continue; // reserved
					const int mpeg_idx = (mpeg_ver == 3) ? 0 : (mpeg_ver == 2) ? 1 : 2;
					native_rate     = sr_table[mpeg_idx][sr_idx];
					native_channels = ((probe[scan + 3] >> 6) & 0x3) == 3 ? 1 : 2; // 0b11=mono
					parsed = true;
				}

				if (!parsed)
				{
					LOG_ERROR("Could not parse MP3 header for %s, assuming 44100Hz stereo\n", filename.c_str());
				}

				// Seek to audio data start (past ID3 tag) so decode_chunk() reads frames.
				fseek(mp3_file, static_cast<long>(data_start_offset), SEEK_SET);

				source_format      = FORMAT_MP3;
				source_channels    = static_cast<uint16_t>(native_channels);
				source_bps         = 16; // AJM outputs S16
				data_size          = 0;  // not used for streaming MP3

				// Allocate the fixed-size streaming read buffer (~10 KB).
				mp3_read_buf.resize(SCE_AJM_DEC_MP3_MAX_INPUT_BUFFER_SIZE);
				mp3_carry_bytes = 0;

				// Initialise the AJM decoder with hardware resampling to TARGET_SAMPLE_RATE.
				if (!mp3_dec.initialize(source_channels, native_rate, TARGET_SAMPLE_RATE))
				{
					LOG_ERROR("Failed to initialize AJM MP3 decoder for %s\n", filename.c_str());
					fclose(mp3_file);
					mp3_file = nullptr;
					return false;
				}

				// Store TARGET_SAMPLE_RATE so resample_audio() in decode_chunk() is a no-op.
				source_sample_rate = TARGET_SAMPLE_RATE;

				resampled_buffer_read_pos = 0;
				resampled_buffer.clear();

				fileName = filename;
				return true;
			}

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
			if (source_format == FORMAT_MP3)
			{
				// Streaming MP3: seek the file back to the start and clear the carry buffer.
				if (mp3_file)
					fseek(mp3_file, static_cast<long>(data_start_offset), SEEK_SET);
				mp3_carry_bytes = 0;
				// current_frame_position is unused for streaming MP3 but reset for consistency.
				current_frame_position = 0;
			}
			else
			{
				const size_t bytes_per_frame = source_channels * (source_bps / 8);
				const size_t total_frames    = bytes_per_frame ? (data_size / bytes_per_frame) : 0;
				current_frame_position = std::min(frame_position, total_frames);
			}

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

			if (source_format == FORMAT_MP3)
			{
				if (!mp3_file)
					return false;

				// Fill the read buffer: carry bytes are already at the front.
				// Read new data from the file to fill the rest of mp3_read_buf.
				const size_t space     = mp3_read_buf.size() - mp3_carry_bytes;
				const size_t new_bytes = fread(mp3_read_buf.data() + mp3_carry_bytes,
				                               1, space, mp3_file);
				const size_t total_valid = mp3_carry_bytes + new_bytes;

				if (total_valid == 0)
				{
					// End of file — loop or stop.
					if (loop.load())
					{
						fseek(mp3_file, static_cast<long>(data_start_offset), SEEK_SET);
						mp3_carry_bytes = 0;
						return decode_chunk(); // retry from the start
					}
					return false;
				}

				// Output buffer for S16 PCM — on the stack (~18 KB, temporary).
				int16_t pcm_buf[SCE_AJM_DEC_MP3_MAX_OUTPUT_BUFFER_SIZE / sizeof(int16_t)];
				size_t   bytes_consumed = 0;
				const uint32_t bytes_produced = mp3_dec.decode(
					mp3_read_buf.data(), total_valid,
					pcm_buf, sizeof(pcm_buf),
					&bytes_consumed);

				// Keep unconsumed bytes at the front of mp3_read_buf for the next call.
				mp3_carry_bytes = total_valid - bytes_consumed;
				if (mp3_carry_bytes > 0)
					std::memmove(mp3_read_buf.data(),
					             mp3_read_buf.data() + bytes_consumed,
					             mp3_carry_bytes);

				if (bytes_produced == 0)
					return false; // no output this round (shouldn't normally happen)

				// Convert S16 interleaved → float.
				const size_t total_samples = bytes_produced / sizeof(int16_t);
				decoded_buffer.resize(total_samples);
				const float scale = 1.0f / 32768.0f;
				for (size_t i = 0; i < total_samples; i++)
					decoded_buffer[i] = pcm_buf[i] * scale;

				// Channel conversion (e.g. mono → stereo).
				std::vector<float> channel_converted;
				convert_channels(decoded_buffer, source_channels, channel_converted);

				// Resample — no-op: AJM already output at TARGET_SAMPLE_RATE.
				resample_audio(channel_converted, source_sample_rate,
				               resampled_buffer, TARGET_SAMPLE_RATE, TARGET_CHANNELS);

				resampled_buffer_read_pos = 0;
				return !resampled_buffer.empty();
			}

			// ---- WAV path ----
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
			sceAudioOutClose(main_handle);
			main_handle = -1; 

			sceAudioOutOutput(pad_handle, nullptr);
			sceAudioOutClose(pad_handle);
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
		void player::audio_thread_main() 
		{
			const size_t stereo_size = BUFFER_FRAMES * 2;
			const size_t mono_size = BUFFER_FRAMES;

			float stereo_mix[stereo_size];
			int16_t stereo_output[stereo_size];
			int16_t mono_output[mono_size];

			while (running)
			{
				// Clear mix buffer
				std::memset(stereo_mix, 0, sizeof(float) * stereo_size);

				bool playing = false;
				{
					std::lock_guard<std::mutex> lock(mutex);
					for (auto& track : tracks) 
					{

						if (track->is_playing())
						{
							playing = true;

							track->read_samples(stereo_mix, BUFFER_FRAMES);
						}
					}

					/// If no tracks are playing, idle
					if (!playing) 
					{
						sceKernelUsleep(1000);
						continue;
					}
				}

				for (size_t i = 0; i < stereo_size; i++)
				{
					float sample = std::clamp(stereo_mix[i], -1.0f, 1.0f);
					stereo_output[i] = static_cast<int16_t>(sample * 32767.0f);
				}

				// Forward raw float mix to any registered voice write callback (e.g. IPort injection).
				if (write_callback)
					write_callback(stereo_mix, BUFFER_FRAMES);

				// Submit to main output
				auto r = sceAudioOutOutput(main_handle, stereo_output);
				if (r < 0)
				{
					LOG_ERROR("sceAudioOutOutput main failed: 0x%08X\n", r);
				}
				// Handle pad output if connected
				// Downmix to mono
				for (size_t i = 0; i < BUFFER_FRAMES; i++)
				{
					float left = stereo_mix[i * 2];
					float right = stereo_mix[i * 2 + 1];
					float mono = (left + right) * 0.7071f; // -3dB to prevent clipping
					mono = std::clamp(mono, -1.0f, 1.0f);
					mono_output[i] = static_cast<int16_t>(mono * 32767.0f);
				}

				// Submit to pad
				if (pad_handle >= 0)
				{
					sceAudioOutOutput(pad_handle, mono_output);
				}
			}
		}

	}
}
