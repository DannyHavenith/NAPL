////////////////////////////////////////////////////////////////////////////////
//
// filefact.cpp - implementation of the file factory
//

#include "stdafx.h"
#include <boost/algorithm/string/predicate.hpp>
#include "samplebl.h"
#include "filetype.h"
#include "aiff.h"
#include "wav.h"
#include "string.h"
#include "raw_file.h"
#include "filefact.h"

block_producer *filefactory::GetBlockProducer( const std::string &filename)
{
	file_type *ft = GetFileType( filename);
	block_producer *producer;

	if (ft)
	{
		producer = ft->MakeBlockProducer( filename.c_str());
	}
	else
	{
		producer = 0;
	}

	return producer;
}

block_sink *filefactory::GetBlockSink( const std::string &filename)
{
	file_type *ft = GetFileType( filename);
	block_sink *sink;

	if (ft)
	{
		sink = ft->MakeBlockSink( filename.c_str());
	}
	else
	{
		sink = 0;
	}

	return sink;
}

file_type *filefactory::GetFileType( const std::string &filename)
{
	static AIFFFile aiff_file_inst;
	static raw_file raw_file_inst;
	static WAVFile wav_file_inst;
	using boost::algorithm::iends_with;

		if (iends_with(filename, ".aif"))
		{
			return &aiff_file_inst;
		}
		else if (iends_with(filename, ".raw"))
		{
			return &raw_file_inst;
		}
		else
		{
			return &wav_file_inst;
		}

}
