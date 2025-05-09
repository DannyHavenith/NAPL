////////////////////////////////////////////////////////////////////////////////
//
// aiff.cpp - implementation of aiff file functions
//
#include "stdafx.h"
#include "aiff.h"

bool ChunkHeader::Stream( FILE *file, const direction &d)
{
	return be_stream( file, ckID, d) &&
		be_stream( file, ckSize, d);
};

bool extended::Stream( FILE *file, const direction &d)
{
	if (d.isoutput())
	{
		unsigned char bytes[10];
		convert_to_IEEE_754( value, bytes);
		return fwrite( bytes, 1, 10, file) != 0;
	}
	else
	{
		unsigned char bytes[10];
		if (fread( bytes, 10, 1, file) != 0)
		{
			value = convert_fr_IEEE_754( bytes);
			return true;
		}
		else return false;
	}
}

block_producer *AIFFFile::MakeBlockProducer( const char *filename)
{
	stream_header h;
	FILE *fp;

	fp = fopen( filename, "rb");
	if (!fp) return NULL;

	Stream( fp, input());

	GetStreamHeader( h);
	return new file_block_producer( fp, h, soundDataChunk.GetDataOffset());
};

void aiff_block_sink::Start()
{
	if (m_pProducer)
	{
		stream_header h;
		m_pProducer->GetStreamHeader( h);
		m_FileObj->SetStreamHeader( h);
		m_FileObj->Stream( m_pFile, streamable::output());

		fseek( m_pFile, m_FileObj->soundDataChunk.GetDataOffset(), SEEK_SET);

		FetchAll();
	}
}

void aiff_block_sink::ReceiveBlock( const sample_block &b)
{
	fwrite( b.m_start, 1,  b.m_end - b.m_start, m_pFile);
};

aiff_block_sink::~aiff_block_sink()
{
	fclose( m_pFile);
}

unsigned long aiff_block_sink::GetArchitecture()
{
	return ARCHITECTURE_AIFF;
}

block_sink *AIFFFile::MakeBlockSink( const char *filename)
{
	FILE *fp;

	fp = fopen( filename, "wb");
	if (!fp) return NULL;

	return new aiff_block_sink( fp, *this);
}
