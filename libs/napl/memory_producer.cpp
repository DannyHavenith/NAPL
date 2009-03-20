#include "stdafx.h"
#include "samplebl.h"
#include "memory_producer.h"


memory_producer::memory_producer( const stream_header &h, unsigned char *memory)
		:m_header( h),
		buffer_ptr( new byte_buffer( memory, h.frame_size() * h.numframes))
{
	// nop
}

block_result memory_producer::RequestBlock( block_consumer &consumer, sampleno start, unsigned long num)
{
	sample_block b( buffer_ptr);
	b.m_start += (start * m_header.frame_size());
	b.m_end = b.m_start + num * m_header.frame_size();

	consumer.ReceiveBlock( b);

	return 0;
}

void memory_producer::GetStreamHeader( stream_header &h)
{
	h = m_header;
}

