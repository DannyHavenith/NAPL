////////////////////////////////////////////////////////////////////////////////
//
// filefact.h - define the file factory
//
#ifndef _FILE_FACTORY_H_
#define _FILE_FACTORY_H_
#include "filetype.h"
#include "samplebl.h"

class filefactory
{
public:
	static block_producer *GetBlockProducer( const char *filename);
	static block_sink *GetBlockSink( const char *filename);

protected:
	static file_type *GetFileType( const char *filename);
};

#endif