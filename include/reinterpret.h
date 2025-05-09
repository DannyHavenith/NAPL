#include "samplebl.h"
#if !defined( REINTERPRET_H)
#define REINTERPRET_H

class reinterpret_mutator : public block_mutator
{
	stream_header m_header;
	stream_header m_original_header;

	sampleno translate( sampleno input)
	{
		return sampleno( (long long)( input) * m_header.frame_size() / m_original_header.frame_size());
	}

public:
	reinterpret_mutator( const stream_header &h)
		: m_header( h)
	{
	}

	sample_block RequestBlock( sampleno start, unsigned long num) override
	{
		return m_pProducer->RequestBlock( translate( start), translate( num));
	};

	void GetStreamHeader( stream_header &h) override
	{
		h = m_header;
	}

	void LinkTo( block_producer_ptr p) override
	{
		block_consumer::LinkTo( std::move( p));
		p->GetStreamHeader( m_original_header);
		size_t total_length = m_original_header.frame_size() * m_original_header.numframes;
		m_header.numframes = sampleno (total_length / m_header.frame_size());
	}

};
#endif //REINTERPRET_H
