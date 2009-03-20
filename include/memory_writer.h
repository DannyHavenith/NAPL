void *write_frames_to_memory( block_producer *p, 
							 void *memory, 
							 sampleno start, 
							 sampleno samples
							 );

void *add_floats_to_memory( block_producer *p,
						   float *memory,
						   sampleno start,
						   sampleno samples
						   );