#include <algorithm>
struct delay_base : public block_mutator, protected block_owner
{
	// maximum blocksize we're willing to spend on a block filled with zeros.
	enum { delay_block_size = 65536};

	delay_base( double delay_time_seconds, size_t samplesize, double post_seconds = 0)
		:m_delay_seconds( delay_time_seconds),
		m_samplesize( samplesize),
		m_post_seconds( post_seconds),
		m_producer_samples( 0),
		block_owner( delay_block_size)
	{
		// nop
	}

	// when asked for a header give that of our producer, but alter the
	// samplecount first
	//
	virtual void GetStreamHeader( stream_header &h)
	{
		m_pProducer->GetStreamHeader( h);


		m_delay_samples = static_cast<size_t>(h.samplerate * m_delay_seconds);
		m_post_size = static_cast<size_t>(h.samplerate * m_post_seconds);

		h.numframes += (m_delay_samples + m_post_size);
	}

	virtual void LinkTo( block_producer *p)
	{
		if (p)
		{
			stream_header h;
			p->GetStreamHeader( h);
			m_producer_samples = h.numframes;
		}
		else
		{
			m_producer_samples = 0;
		}
		block_mutator::LinkTo( p);
	}


	virtual void Seek( sampleno start)
	{
		assert( false);
	}

	virtual block_result RequestBlock( block_consumer &consumer, sampleno start, unsigned long num)
	{

		state_saver<block_consumer *> save( m_pConsumer);
		m_pConsumer = &consumer;

		if (start < m_delay_samples)
		{
			size_t count = m_delay_samples - start;
			if (count > num) count = num;

			send_delay_samples( count, consumer);
			start = 0;
			num -= count;
		}
		else
		{
			start -= m_delay_samples;
		}

		if (num)
		{
			if (start < m_producer_samples)
			{
				size_t count = m_producer_samples - start;
				if (count > num) count = num;

				m_pProducer->RequestBlock( consumer, start, count);
				start = 0;
				num -= count;
			}
			else
			{
				start -= m_producer_samples;
			}

			if (num)
			{
				send_delay_samples( num, consumer);
			}
			return 0;
		}
		else
		{
			return 0;
		}
	}

	void init_block( sample_block &block)
	{
		std::fill( block.buffer_begin(), block.buffer_begin() + block.buffer_size(), 0);
		   //( block.buffer_begin(), 0, block.buffer_size());
	}

	void send_delay_samples( size_t num_samples, block_consumer &consumer)
	{
		block_handle h( this); // releases the block on exit
		sample_block &block( h.get_block());

		if (!h.is_initialized())
		{
			init_block( block);
		}

		block.m_end = block.buffer_begin() + block.buffer_size();

		const size_t samples_in_block = block.buffer_size() / m_samplesize;
		while (num_samples > samples_in_block)
		{
			consumer.ReceiveBlock( block);
			num_samples -= samples_in_block;
		}

		if (num_samples)
		{
			block.m_end = block.buffer_begin() + (num_samples * m_samplesize);
			consumer.ReceiveBlock( block);
		}
	}

	//
	// this will normally never be called.
	virtual void ReceiveBlock( const sample_block &b)
	{
		m_pConsumer->ReceiveBlock( b);
	}

private:
	double m_delay_seconds;
	double m_post_seconds;
	size_t m_delay_samples;
	size_t m_post_size;
	size_t m_producer_samples;
	size_t m_samplesize;
};

template <typename sampletype>
struct delay: public delay_base
{
	delay( double delay_seconds, double post_seconds = 0)
		:delay_base( delay_seconds, sizeof( sampletype), post_seconds)
	{
	};

};
