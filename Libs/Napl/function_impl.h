template <typename sampletype>
struct function : public creative_block_producer 
{

	function( const std::vector< double> proto, const stream_header &h)
		:creative_block_producer( function::buffer_size_stategy( proto.size())),
		m_pos(0),
		m_period( proto.size()),
		m_header(h)
	{
		sample_container< sampletype> container( m_block);
		sample_container< sampletype>::iterator dest = container.begin();

		// for this while loop to work, it is important that the 
		// m_block size is an exact multiple of the proto size
		while (dest != container.end())
		{
			std::vector<double>::const_iterator source;
			for (source = proto.begin(); source != proto.end(); ++source, ++dest)
			{
				sampletraits<sampletype>::channel_type chan;
				denormalize_sample( *source, chan);

				//denormalize_sample( sampletraits<sampletype>::expand_to_channels( *source), *dest);
				*dest =  sampletraits<sampletype>::expand_to_channels( chan);
			}
		}
	}

	virtual void GetStreamHeader( stream_header &h)
	{
		h = m_header;
	}

	virtual void Seek( sampleno pos)
	{
		m_pos = pos;
	}

private:

	stream_header m_header;
	sampleno m_pos;
	sampleno m_period;

	virtual sampleno FillBlock( sample_block &b, sampleno requested)
	{
		sampleno offset = m_pos % m_period;
		b.m_start = b.buffer_begin() + offset * sizeof sampletype;
		b.m_end = b.buffer_begin() + b.buffer_size();

		sampleno in_buffer = (b.m_end - b.m_start) / sizeof sampletype;
		if (in_buffer > requested)
		{
			in_buffer = requested;
			b.m_end = b.m_start + in_buffer * sizeof sampletype;
		}

		return in_buffer;
	}

	// determine how big the buffer will be.
	static size_t buffer_size_stategy( size_t start_size)
	{
		const size_t approximate_size = 500000;
		size_t result = start_size * sizeof sampletype;

		// if the prototype is smaller than 500k make a buffer with size
		// the largest multiple of the proto size that is smaller than 500k
		if (result < approximate_size)
		{
			result = result * (approximate_size / result); 
		}

		return result;
	}
};