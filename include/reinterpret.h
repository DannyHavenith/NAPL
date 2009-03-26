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

	virtual block_result RequestBlock( block_consumer &consumer, sampleno start, unsigned long num)
	{
		return m_pProducer->RequestBlock( consumer, translate( start), translate( num));
	};

	virtual void GetStreamHeader( stream_header &h)
	{
		h = m_header;
	}

	virtual void Seek( sampleno start)
	{
		m_pProducer->Seek( translate( start));
	}

	virtual void LinkTo( block_producer *p)
	{
		m_pProducer = p;
		p->GetStreamHeader( m_original_header);
		size_t total_length = m_original_header.frame_size() * m_original_header.numframes;
		m_header.numframes = sampleno (total_length / m_header.frame_size());
		NotifyProducer();
	}

	virtual void ReceiveBlock( const sample_block &b)
	{
	};
};
#endif //REINTERPRET_H
