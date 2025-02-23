#pragma once

#include "../filesystem.h"

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