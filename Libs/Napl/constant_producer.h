template<typename sample_type>
class constant_producer : public block_producer, private block_owner
{
private:
	stream_header m_header;

	void fill_block()
	{
		sample_container< sample_type> cont( m_block);
		sample_container< sample_type>::iterator i;
		for (i = cont.begin(); i != cont.end(); ++i)
		{
			*i = sampletraits< sample_type>::get_middle();
		}
	}
public:
	constant_producer( stream_header h)
		:m_header( h)
	{
		m_header.architecture = LOCAL_ARCHITECTURE;
		m_header.numchannels = sampletraits<sample_type>::get_num_channels();
		m_header.samplesize = h.samplesize;
		InitBlock( 1024 * 1024);
		fill_block();
	}

	virtual block_result RequestBlock( block_consumer &consumer, sampleno /*ignored*/, unsigned long num)
	{
		while (num* sizeof( sample_type) > (m_block.buffer_size()))
		{
			m_block.m_end = m_block.m_start + sizeof sample_type * (m_block.buffer_size() / sizeof sample_type); 
			consumer.ReceiveBlock( m_block);
			num -= (m_block.buffer_size() / sizeof( sample_type));
		}

		m_block.m_end = m_block.m_start + sizeof( sample_type) * num;
		consumer.ReceiveBlock( m_block);

		return 0;
	}

	virtual void GetStreamHeader( stream_header &h) 
	{
		h = m_header;
	}

	virtual void Seek( sampleno /*ignore*/)
	{
		// nop
	}


};