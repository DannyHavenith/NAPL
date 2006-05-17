class memory_producer : public block_producer
{
private:
	stream_header m_header;
	sample_block::buffer_ptr_type buffer_ptr;

public:
	memory_producer( const stream_header &h, unsigned char *memory);
	virtual block_result RequestBlock( block_consumer &consumer, sampleno start, unsigned long num);
	virtual void GetStreamHeader( stream_header &h);

	virtual void Seek( sampleno /*ignore*/){};


};