////////////////////////////////////////////////////////////////
//
// Wav.cpp - definition of WAV-file functions
//
#include "stdafx.h"
#include "wav.h"

bool WAVHeader::Stream( FILE *file, const direction &d)
{

	return le_stream( file, ckID, d) && 
		le_stream( file, ckSize, d);
};

bool FormatChunk::Stream( FILE *file, const direction &d)
{
	bool bResult = 
		le_stream( file, header, d) &&
		le_stream( file, format, d) &&
		le_stream( file, nChannels, d) &&
		le_stream( file, sampleRate, d) &&
		le_stream( file, bytesPerSecond, d) &&
		le_stream( file, blockAlign, d) &&
		le_stream( file, bitsPerSample, d);
	if (bResult)
	{
		fseek( file, header.ckSize - 16, SEEK_CUR);
	}
	return bResult;
}

bool WAVDataChunk::Stream( FILE *file, const direction &d)
{
	if ( le_stream( file, header, d))
	{
		dataoffset = ftell( file);

		// create a more flexible read:
		// if the data chunk size is zero, it might be that some writing program crashed before it
		// could write the chunk length to the file. To handle this type of data, we assume that the 
		// rest of the file is the data chunk, but only IF the ckSize is zero...
		if ( header.ckSize == 0)
		{
			fseek( file, 0, SEEK_END);
			header.ckSize = ftell( file) - dataoffset;
			fseek( file, dataoffset, SEEK_SET);
		}
		return true;
	}
	else
		return false;
}

block_producer *WAVFile::MakeBlockProducer( const char *filename)
{
	stream_header h;
	FILE *fp;

	fp = fopen( filename, "rb");
	if (!fp) return NULL;

	Stream( fp, input());

	GetStreamHeader( h);
	return new file_block_producer( fp, h, dataChunk.GetDataOffset());
};

bool WAVFile::Stream( FILE *file, const direction &d)
{
	return
		le_stream( file, header, d) &&
		le_stream( file, ID, d) &&
		le_stream( file, formatChunk, d) &&
		le_stream( file, dataChunk, d);
}



void wav_block_sink::Start()
{
	if (m_pProducer)
	{
		stream_header h;
		m_pProducer->GetStreamHeader( h);
		m_FileObj->SetStreamHeader( h);
		m_FileObj->Stream( m_pFile, streamable::output());

		fseek( m_pFile, m_FileObj->dataChunk.GetDataOffset(), SEEK_SET);

		m_pProducer->RequestBlock( *this, 0, h.numframes);
	};
}


void wav_block_sink::ReceiveBlock( const sample_block &b)
{
	fwrite( b.m_start, 1,  b.m_end - b.m_start, m_pFile);
};

wav_block_sink::~wav_block_sink()
{
	fclose( m_pFile);
	delete m_FileObj;
}

block_sink *WAVFile::MakeBlockSink( const char *filename)
{
	FILE *fp;

	fp = fopen( filename, "wb");
	if (!fp) return NULL;

	return new wav_block_sink( fp, *this);
}

