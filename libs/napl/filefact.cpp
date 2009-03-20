////////////////////////////////////////////////////////////////////////////////
//
// filefact.cpp - implementation of the file factory
//

#include "stdafx.h"
#include "samplebl.h"
#include "filetype.h"
#include "aiff.h"
#include "wav.h"
#include "string.h"
#include "raw_file.h"
#include "filefact.h"

block_producer *filefactory::GetBlockProducer( const char *filename)
{
	file_type *ft = GetFileType( filename);
	block_producer *producer;

	if (ft)
	{
		producer = ft->MakeBlockProducer( filename);
	}
	else
	{
		producer = 0;
	}
	
	return producer;
}

block_sink *filefactory::GetBlockSink( const char *filename)
{
	file_type *ft = GetFileType( filename);
	block_sink *sink;

	if (ft)
	{
		sink = ft->MakeBlockSink( filename);
	}
	else
	{
		sink = 0;
	}
	
	return sink;
}

file_type *filefactory::GetFileType( const char *filename)
{
	static AIFFFile aiff_file_inst;
	static raw_file raw_file_inst;
	static WAVFile wav_file_inst;

	if (strchr( filename, '.'))
	{
		if (!strcmpi( strchr( filename, '.'), ".aif"))
		{
			return &aiff_file_inst;
		}
		else if (!strcmpi( strchr( filename, '.'), ".raw"))
		{
			return &raw_file_inst;
		}
		else
		{
			return &wav_file_inst;
		}
	}
	else return 0;
}
