#pragma once

// WAV file utilities - standard includes provided by stdafx.h
// Depends on: filesystem

#include "../file/filesystem.h"

struct riff_header
{
	int chunk_id;
	uint32_t chunk_size;
};

struct riff_chunk
{
	int chunk_id;
	uint32_t chunk_offset;
	uint32_t chunk_size;
};

struct fmt_data
{
	signed short     audio_format;
	unsigned short   num_channels;
	unsigned int     sample_rate;
	unsigned int     byte_rate;
	unsigned short   block_align;
	unsigned short   bits_per_sample;
};

typedef struct  WAV_HEADER
{
	/* RIFF Chunk Descriptor */
	uint8_t         RIFF[4];        // RIFF Header Magic header
	uint32_t        ChunkSize;      // RIFF Chunk Size
	uint8_t         WAVE[4];        // WAVE Header
	/* "fmt" sub-chunk */
	uint8_t         fmt[4];         // FMT header
	uint32_t        Subchunk1Size;  // Size of the fmt chunk
	uint16_t        AudioFormat;    // Audio format 1=PCM,6=mulaw,7=alaw,     257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
	uint16_t        NumOfChan;      // Number of channels 1=Mono 2=Sterio
	uint32_t        SamplesPerSec;  // Sampling Frequency in Hz
	uint32_t        bytesPerSec;    // bytes per second
	uint16_t        blockAlign;     // 2=16-bit mono, 4=16-bit stereo
	uint16_t        bitsPerSample;  // Number of bits per sample
	/* "data" sub-chunk */
	uint8_t         Subchunk2ID[4]; // "data"  string
	uint32_t        Subchunk2Size;  // Sampled data length
} wav_hdr;

class wave
{
public:
	riff_chunk riff;
	riff_chunk fmt;
	riff_chunk data;
	fmt_data format;
	float duration;
	liborbisutil::file_system fs;

	wave() {}
	~wave() {}
	//~WaveFile() { if (fs.is_open()) Close(); }

	bool open(std::string name);
	void close();

	bool read_header();

	int get_format();
	int get_sample_rate();
	int get_channels();

	bool is_mono();

	uint32_t get_data_offset();
	uint32_t get_duration();

	bool read_data(int offset, char* data, size_t dataSize, size_t* read_amount);
	bool read_sample(int position, char* buffer, size_t bufferSize, size_t* read_amount);
	bool find_riff(unsigned int chunkId, liborbisutil::file_system& fs, riff_chunk* chunk);
};