#ifndef RAW_FILE_H
#define RAW_FILE_H

#include "filetype.h"
#include "samplebl.h"

struct raw_file : public file_type
{
    virtual block_sink *MakeBlockSink( const char *filename);
    virtual block_producer *MakeBlockProducer( const char *filename);
};

#endif // RAW_FILE_H
