/***********************************************************************************
**
** Wav.h definitions for Wav file parsing and writing.
**
**
/***********************************************************************************/

// include general file type utility classes.
#include <boost/cstdint.hpp>
#include "filetype.h"

#define ARCHITECTURE_WAV ARCHITECTURE_LITTLEENDIAN

#define ID_RIFF IDword( 'R','I','F','F')
#define ID_WAVE IDword( 'W','A','V','E')
#define ID_FMT  IDword( 'f','m','t',' ')
#define ID_DATA IDword( 'd','a','t','a')


// WAVHeader represents the part that starts each WAV-chunk
struct WAVHeader : public streamable
{
	ChunkID ckID;
	boost::uint32_t ckSize;

	virtual bool Stream( FILE *file, const direction &d);

	WAVHeader( boost::uint32_t chunkID)
	{
		ckID = chunkID;
	}
};

struct FormatChunk: public streamable
{
	WAVHeader header;
	boost::uint16_t format;
	boost::uint16_t nChannels;
	boost::uint32_t sampleRate;
	boost::uint32_t bytesPerSecond;
	boost::uint16_t blockAlign;
	boost::uint16_t bitsPerSample;

	virtual bool Stream( FILE *file, const direction &d);

	FormatChunk(): header( ID_FMT)
	{
		header.ckSize = 16;
		format = 1;
		blockAlign = 1;
	}

	void GetStreamHeader( stream_header &h)
	{
		h.samplesize = bitsPerSample;
		h.samplerate = sampleRate;
		//h.numframes = numSampleFrames;
		h.numchannels = nChannels;
		h.architecture = ARCHITECTURE_WAV;
	}

	bool SetStreamHeader( stream_header &h)
	{
		blockAlign = h.frame_size();
		bitsPerSample = h.samplesize;
		sampleRate = h.samplerate;
		nChannels = h.numchannels;
		bytesPerSecond = (bitsPerSample / 8) * nChannels * sampleRate;
		return true;
	}

};


struct WAVDataChunk: public streamable
{
	WAVHeader header;
	boost::uint32_t dataoffset;

	WAVDataChunk(): header( ID_DATA) {};

	virtual bool Stream( FILE *file, const direction &d);

	boost::uint16_t GetDataOffset()
	{
		return dataoffset;
	}
};

struct WAVFile : public streamable, public file_type
{
	WAVHeader header;
	ChunkID		ID;
	FormatChunk formatChunk;
	WAVDataChunk dataChunk;

	WAVFile(): header( ID_RIFF), ID( ID_WAVE) {};

	inline void CalculateChunkSizes()
	{
		header.ckSize = formatChunk.header.ckSize + dataChunk.header.ckSize + 20;
	}
	inline void GetStreamHeader( stream_header &h)
	{
		formatChunk.GetStreamHeader( h);

		// in WAV, the number of frames is implicit
		if (h.samplesize)
			h.numframes = dataChunk.header.ckSize / ((h.get_sample_size() / 8) * h.numchannels);
		else
			h.numframes = 0;
	}

	inline bool SetStreamHeader( stream_header &h)
	{
		dataChunk.header.ckSize = h.numframes * (h.get_sample_size() / 8) * h.numchannels;
		CalculateChunkSizes();
		return formatChunk.SetStreamHeader( h);

	}

	virtual block_sink *MakeBlockSink( const char *filename);

	virtual block_producer *MakeBlockProducer( const char *filename);

	virtual bool Stream( FILE *file, const direction &d);
};

class wav_block_sink : public block_sink
{
public:
	wav_block_sink( FILE *file, WAVFile &fileobj)
		: m_FileObj( &fileobj)
	{
		m_pFile = file;
	}
	virtual void Start();
	virtual void ReceiveBlock( const sample_block &b);

	virtual ~wav_block_sink();

protected:
	FILE *m_pFile;
	 WAVFile * const m_FileObj;
};


