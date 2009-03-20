struct raw_file : public file_type
{
	virtual block_sink *MakeBlockSink( const char *filename);
	virtual block_producer *MakeBlockProducer( const char *filename);
};