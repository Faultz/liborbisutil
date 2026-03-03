#include "../../stdafx.h"
#include "wav.h"

const uint32_t RIFF_CC = 'RIFF';
const uint32_t FMT_CC = 'fmt ';
const uint32_t DATA_CC = 'data';

bool wave::open(std::string name)
{
	fs.open(name.data(), "rb");

	return fs.is_open();
}

void wave::close()
{
	fs.close();
}


bool wave::read_header()
{
	// seek back to beginning
	fs.seek(0, SEEK_SET);

	bool hasRiff = find_riff(RIFF_CC, fs, &riff);
	bool hasFmt = find_riff(FMT_CC, fs, &fmt);
	bool hasData = find_riff(DATA_CC, fs, &data);

	if (!hasRiff || !hasFmt || !hasData)
		return false;

	fs.seek(fmt.chunk_offset, SEEK_SET);
	fs.read(&format, sizeof(fmt_data));

	double durationInSec = static_cast<double>(data.chunk_size) / (format.num_channels * format.sample_rate * (format.bits_per_sample / 8));
	duration = durationInSec;

	return true;
}

int wave::get_format()
{
	return format.audio_format;
}

int wave::get_sample_rate()
{
	return format.sample_rate;
}

int wave::get_channels()
{
	return format.num_channels;
}

bool wave::is_mono()
{
	return format.num_channels == 1;
}

uint32_t wave::get_data_offset()
{
	return data.chunk_offset;
}

uint32_t wave::get_duration()
{
	return data.chunk_size;
}

bool wave::read_data(int offset, char* data, size_t dataSize, size_t* read_amount)
{
	fs.seek(offset, SEEK_SET);

	int read = fs.read(data, dataSize);
	*read_amount = read;

	return read != 0;
}

bool wave::read_sample(int position, char* buffer, size_t bufferSize, size_t* read_amount)
{
	return read_data(position, buffer, bufferSize, read_amount);
}

bool wave::find_riff(unsigned int chunkId, liborbisutil::file& fs, riff_chunk* chunk)
{
	uint32_t offset = 0;

	chunk->chunk_id = __bswap32(chunkId);

	while (true)
	{
		fs.seek(offset, SEEK_SET);

		int chunkID;
		fs.read((char*)&chunkID, 4);

		if (!fs.is_open())
			return false;

		if (chunkID == chunk->chunk_id)
		{
			chunk->chunk_offset = offset + 8;
			fs.read(reinterpret_cast<char*>(&chunk->chunk_size), 4);

			//chunk->ChunkSize = __bswap32(chunk->ChunkSize);

			return true;
		}

		offset += 1;
	}

	return false;
}