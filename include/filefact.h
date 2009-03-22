////////////////////////////////////////////////////////////////////////////////
//
// filefact.h - define the file factory
//
#ifndef _FILE_FACTORY_H_
#define _FILE_FACTORY_H_
#include <string>
#include "filetype.h"
#include "samplebl.h"

class filefactory
{
public:
	static block_producer *GetBlockProducer( const std::string &filename);
	static block_sink *GetBlockSink( const std::string &filename);

protected:
	static file_type *GetFileType( const std::string &filename);
};

#endif
