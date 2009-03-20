/***********************************************************************************
**
** aiff.h definitions for aiff file parsing and writing.
**
**
/***********************************************************************************/

// include general file type utility classes.
#include "ieee754.h"
#include "filetype.h"

#define ARCHITECTURE_AIFF ARCHITECTURE_BIGENDIAN

#define HEADER_FORM IDword( 'F','O','R','M')
#define HEADER_AIFF IDword( 'A','I','F','F')
#define HEADER_COMM IDword( 'C','O','M','M')
#define HEADER_SSND IDword( 'S','S','N','D')

// ChunkHeader represents the part that starts each AIFF-chunk
struct ChunkHeader : public streamable
{
	ChunkID ckID;
	unsigned long ckSize;

	virtual bool Stream( FILE *file, const direction &d);

	ChunkHeader( unsigned long chunkID)
	{
		ckID = chunkID;
	}
};

///////
//
// extended - represent the 80 bit float type used in AIFF (ieee754)
//
struct extended : public streamable
{
	virtual bool Stream( FILE *file, const direction &d);
	operator double() const { return value;}
	operator unsigned long() { return static_cast<unsigned long>(value);}

	extended( unsigned long source ) { value = static_cast<double>( source);}
	extended( double source) { value = source;}
	extended() {value = 0;}


private:
	double value;
	
};

struct  CommonChunk : public streamable
{
	ChunkHeader header;
	unsigned short	numChannels;
	unsigned long numSampleFrames;
	unsigned short	sampleSize;
	extended sampleRate;

	CommonChunk() : header( HEADER_COMM) 
	{
		header.ckSize = 18;
	};

	void GetStreamHeader( stream_header &h)
	{
		h.samplesize = sampleSize;
		h.samplerate = static_cast< unsigned long>( sampleRate);
		h.numframes = numSampleFrames;
		h.numchannels = numChannels;
		h.architecture = ARCHITECTURE_AIFF;
	}

	bool SetStreamHeader( stream_header &h)
	{
		if (h.architecture != ARCHITECTURE_AIFF) return false;

		sampleSize = h.samplesize;
		sampleRate = h.samplerate;
		numSampleFrames = h.numframes;
		numChannels = h.numchannels;
		return true;
	}

	bool Stream( FILE *file, const direction &d)
	{
		return 
			be_stream( file, header, d) &&
			be_stream( file, numChannels, d) &&
			be_stream( file, numSampleFrames, d) &&
			be_stream( file, sampleSize, d) &&
			be_stream( file, sampleRate, d);
	}
};


struct SoundDataChunk : public streamable
{
	ChunkHeader header;
	unsigned long offset;
	unsigned long blocksize;
	unsigned long dataoffset;

	SoundDataChunk(): header( HEADER_SSND) 
	{
		offset = 0;
		blocksize = 0;
	};

	inline bool Stream( FILE *file, const direction &d)
	{
		if ( 
			be_stream( file, header, d) &&
			be_stream( file, offset, d) &&
			be_stream( file, blocksize, d))
		{
			dataoffset = ftell( file) + offset;
			return true;
		}
		else
			return false;

	}
	unsigned long GetDataOffset()
	{
		return dataoffset;
	}
};


struct AIFFFile : public streamable, public file_type
{
	ChunkHeader header;
	ChunkID		ID;
	CommonChunk commonChunk;
	SoundDataChunk soundDataChunk;

	AIFFFile(): header( HEADER_FORM), ID( HEADER_AIFF) {};

	inline void CalculateChunkSizes()
	{
		header.ckSize = commonChunk.header.ckSize + soundDataChunk.header.ckSize + 20;
	}

	inline void GetStreamHeader( stream_header &h)
	{
		commonChunk.GetStreamHeader( h);
	}

	inline bool SetStreamHeader( stream_header &h)
	{
		soundDataChunk.header.ckSize = h.numframes * (h.get_sample_size()/ 8) * h.numchannels + 8;
		CalculateChunkSizes();
		return commonChunk.SetStreamHeader( h);
	}

	virtual block_sink *MakeBlockSink( const char *filename);

	virtual block_producer *MakeBlockProducer( const char *filename);

	bool Stream( FILE *file, const direction &d)
	{
		return
			be_stream( file, header, d) &&
			be_stream( file, ID, d) &&
			be_stream( file, commonChunk, d) &&
			be_stream( file, soundDataChunk, d);
	}
};

class aiff_block_sink : public block_sink
{
public:
	aiff_block_sink( FILE *file, AIFFFile &fileobj)
		: m_FileObj( &fileobj)
	{
		m_pFile = file;
	}
	virtual void Start();
	virtual void ReceiveBlock( const sample_block &b);

	virtual ~aiff_block_sink();

protected:
	virtual unsigned long GetArchitecture();
	FILE *m_pFile;
	 AIFFFile * const m_FileObj;
};


