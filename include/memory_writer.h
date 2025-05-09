#ifndef MEMORY_WRITER_H
#define MEMORY_WRITER_H

#include "samplebl.h"

void *write_frames_to_memory( block_producer_ptr p,
                             void *memory,
                             sampleno start,
                             sampleno samples
                             );

void *add_floats_to_memory( block_producer_ptr p,
                           float *memory,
                           sampleno start,
                           sampleno samples
                           );

#endif // MEMORY_WRITER_H
