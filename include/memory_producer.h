#ifndef MEMORY_PRODUCER_H
#define MEMORY_PRODUCER_H

class memory_producer : public block_producer
{
private:
	stream_header m_header;
	sample_block::buffer_ptr_type buffer_ptr;

public:
	memory_producer( const stream_header &h, unsigned char *memory);
	sample_block RequestBlock( sampleno start, unsigned long num) override;
	void GetStreamHeader( stream_header &h) override;

};

#endif // MEMORY_PRODUCER_H

